#include "Client.hpp"
#include "HttpRequest.hpp"
#include "Multiplexer.hpp"

ResponseInfos Client::executeCGI(const std::string &cgiPath, const std::string &scriptPath)
{
    //std::cout << "GOT HEEEEERE" << std::endl;
    //std::cout << httpRequest.getMethod()  << std::endl;
    // Generate temporary file names for input and output
    std::string outputFileName = "/tmp/cgi_output_" + generateUniqueString() + ".txt";
    std::string inputFileName = "/tmp/cgi_input_" + generateUniqueString() + ".txt";
    // For POST: Write body to input file before forking
    if (httpRequest.getMethod() == "POST" && !httpRequest.GetBody().empty()){
        const std::string& body = httpRequest.GetBody();
        //std::cout << MAGENTA << body << std::endl;
        std::ofstream inputFile(inputFileName.c_str(), std::ios::out | std::ios::trunc);
        if (!inputFile.is_open()) {
            //std::cout << "Failed to open input file for CGI script" << std::endl;
            throw 500;
        }
        inputFile.write(body.c_str(), body.length());
        inputFile.close();
    }

    pid_t pid = fork();
    if (pid == -1)
        throw INTERNAL; // Internal Server Error

        if (pid == 0) {
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
                // //std::cout << GREEN << "[CGI] Cookie header: " << cookieHeader << COLOR_RESET << std::endl;
                envVars["HTTP_COOKIE"] = cookieHeader;
            }
        
            // Basic CGI vars
            envVars["REQUEST_METHOD"] = httpRequest.getMethod();
            envVars["SCRIPT_FILENAME"] = scriptPath;
            envVars["REDIRECT_STATUS"]  = "200";
            envVars["QUERY_STRING"] = httpRequest.getQueryParams();
        
            // POST-specific vars
            if (httpRequest.getMethod() == "POST") {
                envVars["CONTENT_LENGTH"] = std::to_string(httpRequest.content_length);
                envVars["CONTENT_TYPE"] = httpRequest.GetHeaderContent("content-type");
            }
        
            // Convert env map to envp
            std::vector<std::string> envStrings;
            for (const auto &pair : envVars)
                envStrings.push_back(pair.first + "=" + pair.second);
        
            char *envp[envStrings.size() + 1];
            for (size_t i = 0; i < envStrings.size(); ++i)
                envp[i] = const_cast<char *>(envStrings[i].c_str());
            envp[envStrings.size()] = nullptr;
        
            // Prepare args for execve
            char *argv[] = {const_cast<char *>(cgiPath.c_str()), const_cast<char *>(scriptPath.c_str()), nullptr};
        
            execve(cgiPath.c_str(), argv, envp);
            perror("execve failed");
            // std::exit(1);
            throw INTERNAL;
        }
        
    else
    {


        int status;
        waitpid(pid, &status, 0);

        // Check if CGI process exited normally
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            std::cout << RED << "[CGI ERROR] Script exited abnormally or with error code: "
                    << WEXITSTATUS(status) << COLOR_RESET << std::endl;

            // Optional: log output file contents to debug
            std::ifstream errorFile(outputFileName);
            if (errorFile.is_open()) {
                std::ostringstream errStream;
                errStream << errorFile.rdbuf();
                //std::cout << "[CGI Output Before Crash]:\n" << errStream.str() << std::endl;
                errorFile.close();
            }

            std::remove(outputFileName.c_str());
            std::remove(inputFileName.c_str());

            // Return 500 response
            ResponseInfos errorResponse;
            errorResponse.status = INTERNAL;
            errorResponse.contentType = "text/html";
            errorResponse.body = "<h1>500 Internal Server Error</h1><p>CGI script crashed or failed.</p>";
            errorResponse.headers["Content-Length"] = std::to_string(errorResponse.body.size());
            errorResponse.headers["Content-Type"] = errorResponse.contentType;
            return errorResponse;
        }


        // Read the CGI output
        std::ifstream outputFile(outputFileName);
        if (!outputFile.is_open())
            throw INTERNAL; // Internal Server Error

        std::ostringstream outputBuffer;
        outputBuffer << outputFile.rdbuf();
        outputFile.close();

        std::remove(outputFileName.c_str());
        std::remove(inputFileName.c_str());

        // Parse the CGI output
        std::string cgiOutput = outputBuffer.str();

        size_t headerEnd = cgiOutput.find("\r\n\r\n");
        std::string headerPart, bodyPart;
        if (headerEnd == std::string::npos) {
            // No headers, treat all as body
            headerPart = "";
            bodyPart = cgiOutput;
        } else {
            headerPart = cgiOutput.substr(0, headerEnd);
            bodyPart = cgiOutput.substr(headerEnd + 4);
        }

        // Parse headers
        std::istringstream headerStream(headerPart);
        std::string line;
        std::map<std::string, std::string> headers;
        while (std::getline(headerStream, line) && !line.empty())
        {
            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos)
            {
                std::string key = line.substr(0, colonPos);
                std::string value = line.substr(colonPos + 1);
                // Trim whitespace from value
                value.erase(0, value.find_first_not_of(" \t\r\n"));
                value.erase(value.find_last_not_of(" \t\r\n") + 1);
                if (key == "Set-Cookie") {
                    // Multiple Set-Cookie headers are allowed
                    if (headers.count("Set-Cookie")) {
                        headers["Set-Cookie"] += "\r\nSet-Cookie: " + value;
                    } else {
                        headers["Set-Cookie"] = value;
                    }
                }
                else {
                    headers[key] = value;
                }
                
            }
        }
        // Create the response
        if (headerPart.empty())
        {
            headers["Content-Type"] = "text/html"; // Default content type if not specified
        }
        ResponseInfos response;
        response.status = OK; // Default to 200 OK
        if (headers.find("Status") != headers.end())
        {
            response.status = std::stoi(headers["Status"]);
        }
        response.body = bodyPart;
        response.headers = headers;
        response.contentType = headers["Content-Type"];
        response.headers["Content-Length"] = to_string(response.body.size());
        //std::cout << RED << "gotggggggg" << std::endl;
        return response;

        ///////////////////////////////////////////

        // response.headers[""] = 
        // response.headers[""] = 
        // response.body = 
        // response.status = OK;
        // response.contentType = "text/html";
        // response.
    }
}