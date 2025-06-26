#include "Client.hpp"
#include "HttpRequest.hpp"
#include "Multiplexer.hpp"
#include <signal.h>
#include <unistd.h>
#include <ctime>
#include <set>

static std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    size_t end = str.find_last_not_of(" \t\r\n");
    if (start == std::string::npos || end == std::string::npos)
        return "";
    return str.substr(start, end - start + 1);
}


bool parseCGIHeaders(const std::string& cgi_output, std::map<std::string, std::string>& cgi_headers, std::string& cgi_body) {
    size_t headerEnd = cgi_output.find("\r\n\r\n");
    int flag = 4;
    std::string headerPart;
    if (headerEnd == std::string::npos) {
        headerEnd = cgi_output.find("\n\n");
        flag = 2;
    }
    if (headerEnd != std::string::npos) {
        headerPart = cgi_output.substr(0, headerEnd);
        if (headerEnd + flag < cgi_output.size())
            cgi_body = cgi_output.substr(headerEnd + flag);
        else
            cgi_body.clear();
    } else {
        cgi_body.clear();
        return false;
    }
    
    std::istringstream headerStream(headerPart);
    std::string line;
    std::set<std::string> seen;
    while (std::getline(headerStream, line)) {
        size_t colonPos = line.find(':');
        if (colonPos == std::string::npos)
            continue;
        std::string key = trim(line.substr(0, colonPos));
        std::string value = trim(line.substr(colonPos + 1));
        std::string lower_key = toLower(key);
        
        if (seen.count(lower_key) && lower_key != "set-cookie") {
            if (cgi_headers.count("Status"))
                cgi_headers["Status"] = "502";
            return false;
        }
        seen.insert(lower_key);
        
        if (lower_key == "set-cookie") {
            size_t pos = value.find('=');
            if (pos == std::string::npos)
                return false;
        }
        cgi_headers[key] = value;
    }
    
    if (cgi_headers.count("Status") && cgi_headers["Status"].find("200") == std::string::npos) {
        return false;
    }
    return true;
}

ResponseInfos Client::executeCGI(const std::string &cgiPath, const std::string &scriptPath)
{
    //std::cout << "GOT HEEEEERE" << std::endl;
    //std::cout << httpRequest.getMethod() << std::endl;
    
    // Generate temporary file names for input and output
    std::string outputFileName = "/tmp/cgi_output_" + generateUniqueString() + ".txt";
    std::string inputFileName = "/tmp/cgi_input_" + generateUniqueString() + ".txt";
    
    pid_t childPid = fork();
    if (childPid == -1)
        throw INTERNAL; // Internal Server Error

    if (childPid == 0) {
        // Child process
        // Redirect stdout to output file
        freopen(outputFileName.c_str(), "w", stdout);
    
        // Redirect stdin depending on method
        if (httpRequest.getMethod() == "POST") {
            freopen(inputFileName.c_str(), "r", stdin);
        } else {
            freopen("/dev/null", "r", stdin);
        }
    
        // Set up environment variables
        std::map<std::string, std::string> envVars;
    
        // Handle cookies (case-insensitive access assumed)
        std::string cookieHeader = httpRequest.GetHeaderContent("cookie");  // lowercase
        if (!cookieHeader.empty()) {
            envVars["HTTP_COOKIE"] = cookieHeader;
        }
    
        // Basic CGI vars
        envVars["REQUEST_METHOD"] = httpRequest.getMethod();
        envVars["SCRIPT_FILENAME"] = scriptPath;
        envVars["REDIRECT_STATUS"] = "200";
        envVars["QUERY_STRING"] = httpRequest.getQueryParams();
    
        // POST-specific vars
        if (httpRequest.getMethod() == "POST") {
            std::ostringstream contentLengthStream;
            contentLengthStream << httpRequest.content_length;
            envVars["CONTENT_LENGTH"] = contentLengthStream.str();
            envVars["CONTENT_TYPE"] = httpRequest.GetHeaderContent("content-type");
        }
    
        // Convert env map to envp
        std::vector<std::string> envStrings;
        for (std::map<std::string, std::string>::const_iterator it = envVars.begin(); 
             it != envVars.end(); ++it) {
            envStrings.push_back(it->first + "=" + it->second);
        }
    
        char **envp = new char*[envStrings.size() + 1];
        for (size_t i = 0; i < envStrings.size(); ++i) {
            envp[i] = const_cast<char*>(envStrings[i].c_str());
        }
        envp[envStrings.size()] = NULL;
    
        // Prepare args for execve
        char *argv[] = {const_cast<char*>(cgiPath.c_str()), 
                       const_cast<char*>(scriptPath.c_str()), 
                       NULL};
    
        execve(cgiPath.c_str(), argv, envp);
        perror("execve failed");
        delete[] envp;
        std::exit(1);
    } else {
        // Parent process - handle timeout and monitoring
        
        // For POST: Write body to input file after forking
        if (httpRequest.getMethod() == "POST" && !httpRequest.GetBody().empty()) {
            const std::string& body = httpRequest.GetBody();
            //std::cout << MAGENTA << body << std::endl;
            std::ofstream inputFile(inputFileName.c_str(), std::ios::out | std::ios::trunc);
            if (!inputFile.is_open()) {
                std::cerr << "Failed to open input file for CGI script" << std::endl;
                kill(childPid, SIGKILL);
                std::remove(outputFileName.c_str());
                std::remove(inputFileName.c_str());
                throw 500;
            }
            inputFile.write(body.c_str(), body.length());
            inputFile.close();
        }
        
        // Timeout handling with polling
        time_t startTime = time(NULL);
        int timeout_seconds = 30;
        int status;
        pid_t result;
        
        while (true) {
            result = waitpid(childPid, &status, WNOHANG);
            
            if (result == childPid) {
                // Process completed
                break;
            } else if (result == -1) {
                // Error occurred
                std::cerr << RED << "[CGI ERROR] waitpid failed" << COLOR_RESET << std::endl;
                std::remove(outputFileName.c_str());
                std::remove(inputFileName.c_str());
                throw INTERNAL;
            }
            
            // Check for timeout
            time_t currentTime = time(NULL);
            if (currentTime - startTime >= timeout_seconds) {
                std::cerr << RED << "[CGI ERROR] Script timed out after " << timeout_seconds 
                         << " seconds" << COLOR_RESET << std::endl;
                
                // Kill the child process
                kill(childPid, SIGTERM);
                usleep(100000); // Give it 100ms to terminate gracefully
                kill(childPid, SIGKILL); // Force kill
                waitpid(childPid, &status, 0); // Clean up zombie
                
                // Clean up files
                std::remove(outputFileName.c_str());
                std::remove(inputFileName.c_str());
                
                // Return 504 Gateway Timeout response
                ResponseInfos timeoutResponse;
                timeoutResponse.status = 504; // Gateway Timeout
                timeoutResponse.contentType = "text/html";
                timeoutResponse.body = "<h1>504 Gateway Timeout</h1><p>CGI script timed out.</p>";
                
                std::ostringstream contentLengthStream;
                contentLengthStream << timeoutResponse.body.size();
                timeoutResponse.headers["Content-Length"] = contentLengthStream.str();
                timeoutResponse.headers["Content-Type"] = timeoutResponse.contentType;
                return timeoutResponse;
            }
            
            // Sleep for 100ms before next check
            usleep(100000);
        }

        // Check if CGI process exited normally
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            std::cerr << RED << "[CGI ERROR] Script exited abnormally or with error code: "
                     << WEXITSTATUS(status) << COLOR_RESET << std::endl;

            // Optional: log output file contents to debug
            std::ifstream errorFile(outputFileName.c_str());
            if (errorFile.is_open()) {
                std::ostringstream errStream;
                errStream << errorFile.rdbuf();
                std::cerr << "[CGI Output Before Crash]:\n" << errStream.str() << std::endl;
                errorFile.close();
            }

            std::remove(outputFileName.c_str());
            std::remove(inputFileName.c_str());

            // Return 500 response
            ResponseInfos errorResponse;
            errorResponse.status = INTERNAL;
            errorResponse.contentType = "text/html";
            errorResponse.body = "<h1>500 Internal Server Error</h1><p>CGI script crashed or failed.</p>";
            
            std::ostringstream contentLengthStream;
            contentLengthStream << errorResponse.body.size();
            errorResponse.headers["Content-Length"] = contentLengthStream.str();
            errorResponse.headers["Content-Type"] = errorResponse.contentType;
            return errorResponse;
        }

        // Read the CGI output
        std::ifstream outputFile(outputFileName.c_str());
        if (!outputFile.is_open()) {
            std::remove(outputFileName.c_str());
            std::remove(inputFileName.c_str());
            throw INTERNAL; // Internal Server Error
        }

        std::ostringstream outputBuffer;
        outputBuffer << outputFile.rdbuf();
        outputFile.close();

        std::remove(outputFileName.c_str());
        std::remove(inputFileName.c_str());

        // Parse the CGI output using your header parsing method
        std::string cgiOutput = outputBuffer.str();
        std::map<std::string, std::string> cgi_headers;
        std::string cgi_body;
        
        bool headerParseSuccess = parseCGIHeaders(cgiOutput, cgi_headers, cgi_body);
        
        if (!headerParseSuccess) {
            std::cerr << RED << "[CGI ERROR] Failed to parse CGI headers" << COLOR_RESET << std::endl;
            
            // Return 502 Bad Gateway response
            ResponseInfos errorResponse;
            errorResponse.status = 502; // Bad Gateway
            errorResponse.contentType = "text/html";
            errorResponse.body = "<h1>502 Bad Gateway</h1><p>CGI script returned invalid headers.</p>";
            
            std::ostringstream contentLengthStream;
            contentLengthStream << errorResponse.body.size();
            errorResponse.headers["Content-Length"] = contentLengthStream.str();
            errorResponse.headers["Content-Type"] = errorResponse.contentType;
            return errorResponse;
        }
        
        // Create the response
        ResponseInfos response;
        response.status = OK; // Default to 200 OK
        
        if (cgi_headers.find("Status") != cgi_headers.end()) {
            std::istringstream statusStream(cgi_headers["Status"]);
            statusStream >> response.status;
        }
        
        response.body = cgi_body;
        response.headers = cgi_headers;
        
        // Set content type
        if (cgi_headers.find("Content-Type") != cgi_headers.end()) {
            response.contentType = cgi_headers["Content-Type"];
        } else {
            response.contentType = "text/html"; // Default content type
            response.headers["Content-Type"] = response.contentType;
        }
        
        std::ostringstream contentLengthStream;
        contentLengthStream << response.body.size();
        response.headers["Content-Length"] = contentLengthStream.str();
        
        //std::cout << RED << "gotggggggg" << std::endl;
        return response;
    }
}