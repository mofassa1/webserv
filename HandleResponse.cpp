#include "Multiplexer.hpp"

ResponseInfos Client::generateResponse(ResponseType type, const std::string &path, int statusCode, Client &client)
{
    ResponseInfos response;
    response.status = statusCode;

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
        mimeTypes[".php"] = "text/html";
        mimeTypes[".py"] = "text/html";  
        std::string ext = getFileExtension(path);
        if (mimeTypes.find(ext) != mimeTypes.end())
            response.contentType = mimeTypes[ext];
        else
            response.contentType = "application/octet-stream";
        response.headers["Content-Type"] = response.contentType;
        response.headers["Content-Length"] = to_string(response.body.size());
        break;
    }

    case RESPONSE_ERROR:
    {
        std::string errorPagePath;
        std::ifstream file;
        std::ostringstream ss;
        std::map<unsigned short, std::string> &Error_pages = client.server_matched->GetDefaultERRPages();
        unsigned short statusCode = static_cast<unsigned short>(response.status);
   
        if (Error_pages.count(statusCode))
        {
            errorPagePath = Error_pages[statusCode];
            file.open(errorPagePath.c_str(), std::ios::binary);
        }
        
        if (!file.is_open())
        {
            std::string message = Client::getStatusMessage(response.status); 
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
        std::ostringstream ss;
        ss << "HTTP/1.1 301 Moved Permanently\r\n";
        ss << "Location: " << path << "\r\n";
        ss << "Content-Length: 0\r\n\r\n";

        response.body = ss.str();
        response.headers["Location"] = path;
        response.contentType = ""; 
        break;
    }
    case RESPONSE_DIRECTORY_LISTING:
    {
        DIR *dir = opendir(path.c_str());
        if (!dir)
            throw FORBIDDEN;
        struct dirent *entry;
        std::ostringstream dirContent;

        dirContent << "<html><body><h1>Directory Listing for " << path << "</h1><ul>";
        while ((entry = readdir(dir)) != NULL)
        {
            if (std::string(entry->d_name) == "." || std::string(entry->d_name) == "..")
                continue;
            std::string to_path = client.LocationMatch.path[client.LocationMatch.path.size() - 1] == '/' ? client.LocationMatch.path : client.LocationMatch.path + "/";
            dirContent << "<li><a href=\"" << to_path + entry->d_name << "\">" << entry->d_name << "</a></li>";
        }
        dirContent << "</ul></body></html>";
        closedir(dir);

        response.body = dirContent.str();
        response.contentType = "text/html";
        response.headers["Content-Type"] = response.contentType;
        response.headers["Content-Length"] = to_string(response.body.size());
        break;
    }
    case RESPONSE_DELETE:
    {
        response.contentType = "text/html";
        response.headers["Content-Type"] = response.contentType;
        response.headers["Content-Length"] = to_string(response.body.size());
        break;
    }
    case RESPONSE_CGI_CHECK:
    {
        response.status = statusCode;
        break;
    }
    default:
        throw 500; 
    }
    return response;
}

bool Multiplexer::handelResponse(Client &client, int eventfd, confugParser &config)
{
    (void)config;

    if(client.Response.status == 1337)
            return false;    
    int fd = eventfd;
    const ResponseInfos &response = client.Response;
    std::ostringstream fullResponse;


    fullResponse << "HTTP/1.1 " << response.status << " "
                 << client.getStatusMessage(client.Response.status) << "\r\n";

    for (std::map<std::string, std::string>::const_iterator it = response.headers.begin();
         it != response.headers.end(); ++it)
    {
        fullResponse << it->first << ": " << it->second << "\r\n";
    }

    fullResponse << "\r\n";

    fullResponse << response.body;

    std::string finalOutput = fullResponse.str();
    ssize_t bytesSent = send(fd, finalOutput.c_str(), finalOutput.size(), 0);
    if(bytesSent == -1){
        // std::cout << RED << "ERROR WHILE SENDING RESPONSE " << COLOR_RESET << std::endl;
    }
    return true;
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
    case 504:
        return "Gateway Timeout";

    default:
        return "Unknown Status";
    }
}