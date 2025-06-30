#include "Client.hpp"

bool isHexDigit(char c) {
    return std::isxdigit(static_cast<unsigned char>(c));
}

char hexToChar(char high, char low) {
    return static_cast<char>((std::isdigit(high) ? high - '0' : std::toupper(high) - 'A' + 10) * 16 +
                             (std::isdigit(low) ? low - '0' : std::toupper(low) - 'A' + 10));
}

bool isBadUri(const std::string &uri) {
    
    for (size_t i = 0; i < uri.size(); ++i) {
        if (uri[i] == '\0' || uri[i] < 32)
            return true;
    }
    return false;
}

bool isBadUriTraversal(const std::string &uri) {
    return uri.find("..") != std::string::npos;
}

void parseParams(const std::string &query, std::map<std::string, std::string> Uri) {
    std::stringstream ss(query);
    std::string pair;

    while (std::getline(ss, pair, '&')) {
        size_t pos = pair.find('=');
        if (pos != std::string::npos)
            Uri[pair.substr(0, pos)] = pair.substr(pos + 1);
        else
            Uri[pair] = "";
    }
}

std::string decodePercentEncoding(const std::string &path) {
    std::string decoded;
    for (size_t i = 0; i < path.size(); ++i) {
        if (path[i] == '%') {
            if (i + 2 >= path.size() || !isHexDigit(path[i + 1]) || !isHexDigit(path[i + 2]))
                throw BAD_REQUEST;
            char c = hexToChar(path[i + 1], path[i + 2]);
            if (c < 32 || c > 126)
                throw BAD_REQUEST;
            decoded += c;
            i += 2;
        } else {
            decoded += path[i];
        }
    }
    return decoded;
}

std::string to_string(int value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}


std::string to_lowercase(const std::string& input){
    std::string result = input;
    for (size_t i = 0; i < result.length(); ++i) {
        if (result[i] >= 'A' && result[i] <= 'Z')
            result[i] += 32;
    }
    return result;
}

bool AreYouNew(int client_sockfd, std::map<int, Client> &clients)
{
    return clients.find(client_sockfd) == clients.end();
}

void epoll_change(int &EpoleFd, int &eventFd)
{
    struct epoll_event event;
    event.events = EPOLLOUT | EPOLLET;
    event.data.fd = eventFd;

    epoll_ctl(EpoleFd, EPOLL_CTL_MOD, eventFd, &event);
}

std::string HOST_AND_PORT(std::string HOST, int PORT){
    std::ostringstream oss;
    oss << PORT;  
    std::string str = oss.str();
    std::string host_and_port = HOST + ":" + str;

    return host_and_port;
}

std::string toLower(const std::string &str)
{
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

std::string generateUniqueString()
{
    std::stringstream ss;
    std::srand(std::time(0) + rand()); 
    ss << std::time(0);                
    ss << "_";
    ss << rand() % 100000;
    return ss.str();
}

std::string getFileExtension(const std::string &path)
{
    size_t dotPos = path.rfind('.');
    if (dotPos != std::string::npos)
        return path.substr(dotPos);
    return "";
}

std::string getExtensionFromContentType(const std::string &contentType)
{
    std::map<std::string, std::string> contentTypeToExtension;

    contentTypeToExtension["text/html"] = ".html";
    contentTypeToExtension["text/css"] = ".css";
    contentTypeToExtension["text/plain"] = ".txt";
    contentTypeToExtension["text/javascript"] = ".js";
    contentTypeToExtension["text/xml"] = ".xml";

    contentTypeToExtension["application/json"] = ".json";
    contentTypeToExtension["application/javascript"] = ".js";
    contentTypeToExtension["application/pdf"] = ".pdf";
    contentTypeToExtension["application/zip"] = ".zip";
    contentTypeToExtension["application/xml"] = ".xml";
    contentTypeToExtension["application/octet-stream"] = ".bin";

    contentTypeToExtension["image/jpeg"] = ".jpg";
    contentTypeToExtension["image/jpg"] = ".jpg";
    contentTypeToExtension["image/png"] = ".png";
    contentTypeToExtension["image/gif"] = ".gif";
    contentTypeToExtension["image/svg+xml"] = ".svg";
    contentTypeToExtension["image/x-icon"] = ".ico";
    contentTypeToExtension["image/bmp"] = ".bmp";
    contentTypeToExtension["image/webp"] = ".webp";

    contentTypeToExtension["video/mp4"] = ".mp4";
    contentTypeToExtension["video/avi"] = ".avi";
    contentTypeToExtension["video/quicktime"] = ".mov";
    contentTypeToExtension["video/x-msvideo"] = ".avi";

    contentTypeToExtension["audio/mpeg"] = ".mp3";
    contentTypeToExtension["audio/wav"] = ".wav";
    contentTypeToExtension["audio/ogg"] = ".ogg";

    std::string mainType = contentType;
    size_t semicolonPos = contentType.find(';');
    if (semicolonPos != std::string::npos)
    {
        mainType = contentType.substr(0, semicolonPos);
    }

    size_t start = mainType.find_first_not_of(" \t");
    size_t end = mainType.find_last_not_of(" \t");
    if (start != std::string::npos && end != std::string::npos)
    {
        mainType = mainType.substr(start, end - start + 1);
    }

    std::map<std::string, std::string>::const_iterator it = contentTypeToExtension.find(mainType);
    if (it != contentTypeToExtension.end())
    {
        return it->second;
    }

    return ".bin";
}

bool isCGI(const std::string &fileExtension, const std::map<std::string, std::string> &cgiMap)
{

    return cgiMap.find(fileExtension) != cgiMap.end();
}

std::string validateUploadDir(const std::string& path) {
    if (path.find("..") != std::string::npos)
        throw BAD_REQUEST;
    return (path == "./") ? "" : path;
}

int countWords(const std::string& input) {
    std::istringstream stream(input);
    std::string word;
    int count = 0;

    while (stream >> word)
        ++count;

    return count;
}