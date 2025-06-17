#include "Client.hpp"

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

std::string getExtensionFromContentType(const std::string &contentType)
{
    // Map of content types to file extensions
    std::map<std::string, std::string> contentTypeToExtension;

    // Text types
    contentTypeToExtension["text/html"] = ".html";
    contentTypeToExtension["text/css"] = ".css";
    contentTypeToExtension["text/plain"] = ".txt";
    contentTypeToExtension["text/javascript"] = ".js";
    contentTypeToExtension["text/xml"] = ".xml";

    // Application types
    contentTypeToExtension["application/json"] = ".json";
    contentTypeToExtension["application/javascript"] = ".js";
    contentTypeToExtension["application/pdf"] = ".pdf";
    contentTypeToExtension["application/zip"] = ".zip";
    contentTypeToExtension["application/xml"] = ".xml";
    contentTypeToExtension["application/octet-stream"] = ".bin";

    // Image types
    contentTypeToExtension["image/jpeg"] = ".jpg";
    contentTypeToExtension["image/jpg"] = ".jpg";
    contentTypeToExtension["image/png"] = ".png";
    contentTypeToExtension["image/gif"] = ".gif";
    contentTypeToExtension["image/svg+xml"] = ".svg";
    contentTypeToExtension["image/x-icon"] = ".ico";
    contentTypeToExtension["image/bmp"] = ".bmp";
    contentTypeToExtension["image/webp"] = ".webp";

    // Video types
    contentTypeToExtension["video/mp4"] = ".mp4";
    contentTypeToExtension["video/avi"] = ".avi";
    contentTypeToExtension["video/quicktime"] = ".mov";
    contentTypeToExtension["video/x-msvideo"] = ".avi";

    // Audio types
    contentTypeToExtension["audio/mpeg"] = ".mp3";
    contentTypeToExtension["audio/wav"] = ".wav";
    contentTypeToExtension["audio/ogg"] = ".ogg";

    // Extract the main content type (remove charset, boundary, etc.)
    std::string mainType = contentType;
    size_t semicolonPos = contentType.find(';');
    if (semicolonPos != std::string::npos)
    {
        mainType = contentType.substr(0, semicolonPos);
    }

    // Trim whitespace
    size_t start = mainType.find_first_not_of(" \t");
    size_t end = mainType.find_last_not_of(" \t");
    if (start != std::string::npos && end != std::string::npos)
    {
        mainType = mainType.substr(start, end - start + 1);
    }

    // Look up the extension
    std::map<std::string, std::string>::const_iterator it = contentTypeToExtension.find(mainType);
    if (it != contentTypeToExtension.end())
    {
        return it->second;
    }

    // Default fallback
    return ".bin";
}

std::string generateUniqueString()
{
    std::stringstream ss;
    std::srand(std::time(0) + rand()); // seed
    ss << std::time(0);                // timestamp
    ss << "_";
    ss << rand() % 100000; // random 5-digit number
    return ss.str();       // e.g., "1718452193_12345"
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
    {
        std::cerr << RED << "HAMOUDA 1" << COLOR_RESET << std::endl;
        throw 404;
    }
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
    LocationMatch.redirect_path = BestMatch.GetPats()["redirect:"];
    if (httpRequest.getMethod() == "POST")
    {
        // GET FINAL URL
        LocationMatch.upload_directory = BestMatch.GetPats()["upload_directory:"];
        if (LocationMatch.upload_directory.empty())
        {
            std::cerr << RED << "HAMOUDA 2"  << COLOR_RESET << std::endl;     
            throw NOT_FOUND;
        }
        std::string file_extension = getExtensionFromContentType(httpRequest.GetHeaderContent("Content-Type"));
        LocationMatch.upload_path = LocationMatch.directory + LocationMatch.path + "/" + LocationMatch.upload_directory + "/" + generateUniqueString() + file_extension;
        LocationMatch.upload_file.open(LocationMatch.upload_path.c_str(), std::ios::binary);

        if (!LocationMatch.upload_file.is_open())
        {
            std::cerr << RED << "HAMOUDA 3" << COLOR_RESET << std::endl;
            throw NOT_FOUND;
        }
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
        LocationCheck();
        std::cout << GREEN << "[" << fd << "]" << "- - - - - - VALID HEADERS - - - - - -" << COLOR_RESET << std::endl;
        if (httpRequest.getMethod() == "POST" && httpRequest.validbody(buffer, server->Getclient_body_size_limit()))
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
        httpRequest.parsebody(buffer, _Readed, BytesReaded, LocationMatch.upload_file);
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
        throw NOT_FOUND;

    if (S_ISDIR(file_info.st_mode))
    {
        if (!LocationMatch.redirect_path.empty())
        {
            std::string redir_path_send;
            if (LocationMatch.redirect_path[LocationMatch.redirect_path.length() - 1] != '/')
                redir_path_send = LocationMatch.redirect_path + "/";
            return generateResponse(RESPONSE_REDIRECT, redir_path_send, 301, LocationMatch);
        }
        if (!LocationMatch.index_file.empty())
        {
            std::string index_path = full_path + "/" + LocationMatch.index_file;

            struct stat index_info;
            if (stat(index_path.c_str(), &index_info) == 0 && S_ISREG(index_info.st_mode))
                return generateResponse(RESPONSE_FILE, index_path, 200, LocationMatch);
        }
        if (LocationMatch.autoindex)
            return generateResponse(RESPONSE_DIRECTORY_LISTING, full_path, 200, LocationMatch);
        else
            throw FORBIDDEN; // forbidden
    }
    if (S_ISREG(file_info.st_mode))
    {
        if (access(full_path.c_str(), R_OK) != 0)
            throw FORBIDDEN; // Forbidden

        return generateResponse(RESPONSE_FILE, full_path, 200, LocationMatch);
    }
    throw NOT_FOUND;
}

ResponseInfos Client::deleteDir(const std::string path)
{
    ResponseInfos response;
    std::ostringstream ss;

    ss << "<html><body><h1>File Deleted</h1><p>" << path << " was successfully deleted.</p></body></html>";
    
    response.status = 200;
    response.body = ss.str();
    response.contentType = "text/html";
    response.headers["Content-Type"] = "text/html";
    response.headers["Content-Length"] = to_string(response.body.size());

    return response;

    // to be handled
   
    //     DIR *dir = opendir(path.c_str());
//     if (!dir)
//         throw FORBIDDEN;

//     struct dirent *entry;
//     while ((entry = readdir(dir)) != NULL)
//     {
//         if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
//         {
//             std::string fullPath = path + "/" + entry->d_name;
//             struct stat statbuf;
//             if (stat(fullPath.c_str(), &statbuf) == -1)
//             {
//                 closedir(dir);
//                 throw FORBIDDEN;
//             }
//             if (S_ISDIR(statbuf.st_mode))
//             {
//                 ResponseInfos resp = deleteDir(fullPath);
//                 if (resp.status != NO_CONTENT)
//                 {
//                     closedir(dir);
//                     return resp; 
//                 }
//             }
//             else
//             {
//                 if (remove(fullPath.c_str()) != 0)
//                 {
//                     closedir(dir);
//                     throw FORBIDDEN; 
//                 }
//             }
//         }
//     }
//     closedir(dir);
//     if (rmdir(path.c_str()) == 0)
    return generateResponse(RESPONSE_DELETE, path, 200, LocationMatch);

//     throw FORBIDDEN;
}


ResponseInfos  Client::DELETE()
{
    ResponseInfos response;
    struct stat statbuf;
    std::string full_path = LocationMatch.directory + LocationMatch.path; // Build the full file path

    if (stat(full_path.c_str(), &statbuf) != 0)
        throw NOT_FOUND;
    if (S_ISREG(statbuf.st_mode))
    {
        if (std::remove(full_path.c_str()) != 0)
            throw FORBIDDEN;

        std::ostringstream ss;
        ss << "<html><body><h1>File Deleted</h1><p>" << full_path << " was successfully deleted.</p></body></html>";
        response.status = 200;
        response.body = ss.str();
    }
    else if (S_ISDIR(statbuf.st_mode))
    {
        // You may want to restrict directory deletion
        return deleteDir(full_path);
        throw FORBIDDEN;
    }
    else
    {
        throw FORBIDDEN;
    }
    response.contentType = "text/html";
    response.headers["Content-Type"] = "text/html";
    response.headers["Content-Length"] = to_string(response.body.size());

    return response;
}


ResponseInfos Client::POST()
{
    ResponseInfos response;

    std::ostringstream ss;
    ss << "<html><body><h1>File uploaded successfully</h1>";
    ss << "<p>Saved to: " << LocationMatch.upload_path << "</p>";
    ss << "</body></html>";

    response.status = 200;
    response.body = ss.str();
    response.contentType = "text/html";
    response.headers["Content-Type"] = "text/html";
    response.headers["Content-Length"] = to_string(response.body.size());

    return response;
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
        mimeTypes[".py"] = "text/html";  // for CGI
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

        if (LocationMatch.Error_pages.count(statusCode))
        {
            errorPagePath = LocationMatch.Error_pages[statusCode];
            file.open(errorPagePath.c_str(), std::ios::binary);
        }
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
        std::ostringstream ss;
        ss << "HTTP/1.1 301 Moved Permanently\r\n";
        ss << "Location: " << path << "\r\n";
        ss << "Content-Length: 0\r\n\r\n";

        response.body = ss.str();
        response.headers["Location"] = path;
        response.contentType = ""; // usually empty for redirects
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
            dirContent << "<li><a href=\"" << entry->d_name << "\">" << entry->d_name << "</a></li>";
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