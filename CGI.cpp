#include "Multiplexer.hpp"

ResponseInfos Client::executeCGI(const std::string &cgiPath, const std::string &scriptPath)
{
    std::cout << "GOT HEEEEERE" << std::endl;
    // Generate temporary file names for input and output
    std::string outputFileName = "/tmp/cgi_output_" + generateUniqueString() + ".txt";
    std::string inputFileName = "/tmp/cgi_input_" + generateUniqueString() + ".txt";
    // // For POST: Write body to input file before forking
    // if (httpRequest.getMethod() == "POST" && !httpRequest.body.empty())
    // {
    //     std::ofstream inputFile(inputFileName);
    //     if (!inputFile.is_open())
    //         throw INTERNAL; // Internal Server Error
    //     inputFile << httpRequest.body;
    //     inputFile.close();
    // }

    pid_t pid = fork();
    if (pid == -1)
        throw INTERNAL; // Internal Server Error

    if (pid == 0)
    {
        // Child process
        freopen(outputFileName.c_str(), "w+", stdout); // Redirect stdout to output file

        // For POST: redirect stdin to input file, for GET: /dev/null
        if (httpRequest.getMethod() == "POST")
            freopen(inputFileName.c_str(), "r", stdin);   // Redirect stdin to input file
        else
            freopen("/dev/null", "r", stdin); // No body for GET

        // Set up environment variables
        std::map<std::string, std::string> envVars;
        envVars["REQUEST_METHOD"] = httpRequest.getMethod();
        envVars["SCRIPT_FILENAME"] = scriptPath;

        // const std::map<std::string, std::string>& params = httpRequest.getQueryParams();
        // std::string queryString;
        // for (std::map<std::string, std::string>::const_iterator it = params.begin(); it != params.end(); ++it) {
        //     if (it != params.begin())
        //         queryString += "&";
        //     queryString += it->first + "=" + it->second;
        // }
        envVars["REDIRECT_STATUS"]  = "200";
        envVars["QUERY_STRING"] = httpRequest.getQueryParams();
        std::cout << RED << "QuEEEEEEEEEEEERY: " << envVars["QUERY_STRING"] << COLOR_RESET << std::endl;

        // POST-specific env vars
        if (httpRequest.getMethod() == "POST") {
            envVars["CONTENT_LENGTH"] = std::to_string(httpRequest.content_length);
            // envVars["CONTENT_TYPE"] = httpRequest.getHeaderContent("Content-Type");
        }

        std::vector<std::string> envStrings;
        for (const auto &pair : envVars)
            envStrings.push_back(pair.first + "=" + pair.second);

        char *envp[envStrings.size() + 1];
        for (size_t i = 0; i < envStrings.size(); ++i)
            envp[i] = const_cast<char *>(envStrings[i].c_str());
        envp[envStrings.size()] = nullptr;

        char *argv[] = {const_cast<char *>(cgiPath.c_str()), const_cast<char *>(scriptPath.c_str()), nullptr};
        execve(cgiPath.c_str(), argv, envp);

        perror("execve failed");
            throw INTERNAL;
        std::exit(1);
    }
    else
    {

        int status;
        waitpid(pid, &status, 0);

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
                headers[key] = value;
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
        std::cout << RED << "gotggggggg" << std::endl;
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