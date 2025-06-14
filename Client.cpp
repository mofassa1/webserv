#include "Client.hpp"
#include <sys/wait.h>
Client::Client() : state(waiting), BytesReaded(0), match_found(false)
{ // Default constructor
    struct timeval tv;
    gettimeofday(&tv, NULL);
    lastTime = (tv.tv_sec * 1000L) + (tv.tv_usec / 1000L);


    // std::cout << "Client default constructor called" << std::endl;
}

Client::~Client()
{
    // std::cout << "Client destructor called" << std::endl;
}

void Client::LocationCheck()
{
    LocationMatch.path = httpRequest.getDecodedPath();
    const std::vector<route> &routes = server->GetRoute();

    size_t best_match_len = 0;

    for (size_t i = 0; i < routes.size(); ++i)
    {
        std::string route_path = server->GetRoute()[i].GetPats()["path:"];
        if (LocationMatch.path.compare(0, route_path.length(), route_path) == 0 &&
            (route_path.length() > best_match_len))
        {
            BestMatch = routes[i];
            best_match_len = route_path.length();
            match_found = true;
        }
    }
    if (!match_found)
        throw 404;
    LocationMatch.methods = BestMatch.GetMethods();
    bool method_allowed = false;
    for (size_t i = 0; i < LocationMatch.methods.size(); ++i)
    {
        if (LocationMatch.methods[i] == httpRequest.getMethod())
        {
            method_allowed = true;
            break;
        }
    }

    if (!method_allowed)
        throw 405;
    LocationMatch.directory = BestMatch.GetPats()["directory:"];
    LocationMatch.autoindex = BestMatch.GetAutoIndex();
    LocationMatch.index_file = BestMatch.GetPats()["index_file:"];
    LocationMatch.cgi = BestMatch.GetCGI();
    LocationMatch.Error_pages = server->GetDefaultERRPages();
    if (httpRequest.getMethod() == "POST")
    {
        // GET FINAL URL
        LocationMatch.upload_directory = BestMatch.GetPats()["upload_directory:"];
        if (LocationMatch.upload_directory.empty())
            throw 700;
        // upload PATH
    }
}

void Client::parse_request(int fd, size_t _Readed)
{
    switch (state)
    {
    case waiting:
        std::cout << GREEN << "[" << fd << "]" << " WAITING" << COLOR_RESET << std::endl;
        if (!httpRequest.VALID_CRLN_CRLN(buffer))
            break;
        state = request_start_line;
        httpRequest.storethebuffer(buffer);
        /* fall through */
    case request_start_line:
        httpRequest.start_line();
        std::cout << GREEN << "[" << fd << "]" << "- - - - - - VALID START LINE - - - - - - -" << COLOR_RESET << std::endl;
        state = request_headers;
        /* fall through */
    case request_headers:
        httpRequest.headers();
        std::cout << GREEN << "[" << fd << "]" << "- - - - - - VALID HEADERS - - - - - -" << COLOR_RESET << std::endl;
        if (httpRequest.getMethod() == "POST" && httpRequest.validbody(buffer))
        {
            std::cout << GREEN << "[" << fd << "]" << "- - - - - - VALID BODY - - - - - - " << COLOR_RESET << std::endl;
            state = request_body;
        }
        else
        {
            state = done;
            break;
        }
        /* fall through */
    case request_body:
        httpRequest.parsebody(buffer, _Readed, BytesReaded);
        if (httpRequest.chunk_done == true)
            state = done;
        break;
    default:
        break;
    }
}

ResponseInfos Client::GET()
{
    std::string full_path = LocationMatch.directory + LocationMatch.path; // Build the full file path

    struct stat file_info;
    if (stat(full_path.c_str(), &file_info) != 0)
        throw 888;

    if (S_ISDIR(file_info.st_mode))
    {
        if (!LocationMatch.index_file.empty())
        {
            std::string index_path = full_path + LocationMatch.index_file;

            struct stat index_info;
            if (stat(index_path.c_str(), &index_info) == 0 && S_ISREG(index_info.st_mode))
                return generateResponse(RESPONSE_FILE, index_path, 200, LocationMatch);
        }
        if (LocationMatch.autoindex)
            return generateResponse(RESPONSE_DIRECTORY_LISTING, full_path, 200, LocationMatch);
        else
            throw 403; // forbidden
    }
    if (S_ISREG(file_info.st_mode))
    {
        std::string file_extension = getFileExtension(full_path);
        if (LocationMatch.cgi.find(file_extension) != LocationMatch.cgi.end())
        {
            std::string cgi_path = LocationMatch.cgi[file_extension];
            return executeCGI(cgi_path, full_path);
        }

        if (access(full_path.c_str(), R_OK) != 0)
            throw 403; // Forbidden

        return generateResponse(RESPONSE_FILE, full_path, 200, LocationMatch);
    }
    throw 889;
}

ResponseInfos Client::executeCGI(const std::string &cgi_path, const std::string &script_path)
{
    int pipefd[2];
    if (pipe(pipefd) == -1)
        throw 500; // Internal Server Error

    pid_t pid = fork();
    if (pid == -1)
        throw 500; // Internal Server Error

    if (pid == 0)
    {
        // Child process
        close(pipefd[0]); // Close read end
        dup2(pipefd[1], STDOUT_FILENO); // Redirect stdout to pipe
        close(pipefd[1]);

        char *env[] = {
            const_cast<char *>("REQUEST_METHOD=GET"),
            const_cast<char *>(std::string("SCRIPT_FILENAME=" + script_path).c_str()),
            NULL};

        execl(cgi_path.c_str(), cgi_path.c_str(), NULL, env);
        exit(1); // If execl fails
    }
    else
    {
        // Parent process
        close(pipefd[1]); // Close write end
        char buffer[1024];
        ssize_t bytesRead;
        std::ostringstream response_body;

        while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0)
        {
            response_body.write(buffer, bytesRead);
        }

        close(pipefd[0]);
        waitpid(pid, NULL, 0); // Wait for child process to finish

        ResponseInfos response;
        response.status = 200;
        response.body = response_body.str();
        response.contentType = "text/html";
        response.headers["Content-Type"] = response.contentType;
        response.headers["Content-Length"] = to_string(response.body.size());
        return response;
    }
}

Client::Client(const Client &other)
{
    lastTime = other.lastTime;
    state = other.state;
    httpRequest = other.httpRequest;
    server = other.server; // Shallow copy
    buffer = other.buffer;
    BytesReaded = other.BytesReaded;
}

Client &Client::operator=(const Client &other)
{
    if (this != &other)
    {
        lastTime = other.lastTime;
        state = other.state;
        httpRequest = other.httpRequest;
        server = other.server; // Shallow copy
        buffer = other.buffer;
        BytesReaded = other.BytesReaded;
    }
    return *this;
}

std::string getFileExtension(const std::string &path)
{
    size_t dotPos = path.rfind('.');
    if (dotPos != std::string::npos)
        return path.substr(dotPos);
    return "";
}

ResponseInfos Client::generateResponse(ResponseType type, const std::string &path, int statusCode, S_LocationMatch &LocationMatch)
{
    ResponseInfos response;
    response.status = statusCode;

    (void)LocationMatch;
    switch (type)
    {
    case RESPONSE_FILE:
    {
        std::ifstream file(path.c_str(), std::ios::binary);
        if (!file)
            throw 404;

        std::ostringstream ss;
        ss << file.rdbuf();
        response.body = ss.str();
        std::map<std::string, std::string> mimeTypes;
        mimeTypes[".html"] = "text/html";
        mimeTypes[".htm"] = "text/html";
        mimeTypes[".css"] = "text/css";
        mimeTypes[".js"] = "application/javascript";
        mimeTypes[".json"] = "application/json";
        mimeTypes[".png"] = "image/png";
        mimeTypes[".jpg"] = "image/jpeg";
        mimeTypes[".jpeg"] = "image/jpeg";
        mimeTypes[".gif"] = "image/gif";
        mimeTypes[".svg"] = "image/svg+xml";
        mimeTypes[".ico"] = "image/x-icon";
        mimeTypes[".mp4"] = "video/mp4";
        mimeTypes[".pdf"] = "application/pdf";
        mimeTypes[".txt"] = "text/plain";
        mimeTypes[".php"] = "text/html"; // for CGI
        mimeTypes[".py"]  = "text/html"; // for CGI
        std::string ext = getFileExtension(path);
        if (mimeTypes.find(ext) != mimeTypes.end())
            response.contentType = mimeTypes[ext];
        else
            response.contentType = "application/octet-stream"; // fallback for unknown types
        response.headers["Content-Type"] = response.contentType;
        response.headers["Content-Length"] = to_string(response.body.size());
        break;
    }

    case RESPONSE_ERROR:
    {
        std::string errorPagePath;
        std::ifstream file;
        std::ostringstream ss;
        unsigned short statusCode = static_cast<unsigned short>(response.status);

        // Check if a custom error page is defined for the status code
        if (LocationMatch.Error_pages.count(statusCode))
        {
            errorPagePath = LocationMatch.Error_pages[statusCode];
            file.open(errorPagePath.c_str(), std::ios::binary);
        }
        // If not found or not openable, fallback to default content
        if (!file.is_open())
        {
            std::string message = Client::getStatusMessage(response.status); // like "Not Found"
            ss << "<html><head><title>" << response.status << " " << message << "</title></head>";
            ss << "<body><h1>" << response.status << " - " << message << "</h1></body></html>";
            response.body = ss.str();
        }
        else
        {
            ss << file.rdbuf();
            response.body = ss.str();
        }

        response.contentType = "text/html";
        response.headers["Content-Type"] = response.contentType;
        response.headers["Content-Length"] = to_string(response.body.size());
        break;
    }

    case RESPONSE_REDIRECT:
    {
        break;
    }

    case RESPONSE_DIRECTORY_LISTING:
    {
        // Just a demo, real version should dynamically generate a listing of files
        // response.body = "<html><body><h1>Directory Listing</h1><ul><li>file1.txt</li><li>file2.txt</li></ul></body></html>";
        // response.contentType = "text/html";
        // response.headers["Content-Type"] = "text/html";
        // response.headers["Content-Length"] = std::to_string(response.body.size());
        break;
    }
    default:
        throw 500; // Internal server error
    }
    return response;
}

std::string Client::getStatusMessage(int statusCode)
{
    switch (statusCode)
    {
    case 200:
        return "OK";
    case 201:
        return "Created";
    case 202:
        return "Accepted";
    case 204:
        return "No Content";

    case 301:
        return "Moved Permanently";
    case 302:
        return "Found";
    case 303:
        return "See Other";
    case 304:
        return "Not Modified";

    case 400:
        return "Bad Request";
    case 401:
        return "Unauthorized";
    case 403:
        return "Forbidden";
    case 404:
        return "Not Found";
    case 405:
        return "Method Not Allowed";
    case 408:
        return "Request Timeout";

    case 500:
        return "Internal Server Error";
    case 501:
        return "Not Implemented";
    case 502:
        return "Bad Gateway";
    case 503:
        return "Service Unavailable";

    default:
        return "Unknown Status";
    }
}