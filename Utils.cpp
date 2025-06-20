#include "Client.hpp"

bool isHexDigit(char c) {
    return std::isxdigit(static_cast<unsigned char>(c));
}

char hexToChar(char high, char low) {
    return static_cast<char>((std::isdigit(high) ? high - '0' : std::toupper(high) - 'A' + 10) * 16 +
                             (std::isdigit(low) ? low - '0' : std::toupper(low) - 'A' + 10));
}

bool isBadUri(const std::string &uri) {
    // Rejects null bytes or control characters
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

#include <string>

std::string to_lowercase(const std::string& input){
    std::string result = input;
    for (size_t i = 0; i < result.length(); ++i) {
        if (result[i] >= 'A' && result[i] <= 'Z')
            result[i] += 32;
    }
    return result;
}