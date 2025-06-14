#pragma once

#include "confugParser.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <algorithm>
#include <sys/time.h>
#include <string>
#include "HttpRequest.hpp"

#define BUFFERSIZE 1000

enum ResponseType {
    RESPONSE_FILE,
    RESPONSE_ERROR,
    RESPONSE_REDIRECT,
    RESPONSE_DIRECTORY_LISTING
};

struct ResponseInfos {
    int status;
    std::string body;
    std::string contentType;
    std::map<std::string, std::string> headers;
};


enum Client_state{
    waiting,
    request_start_line,
    request_headers,
    request_body,
    done
};

typedef struct S_LocationMatch {
    std::string path;
    std::string directory;
    std::vector<std::string> methods;
    std::string index_file;
    std::string upload_directory;
    std::map<std::string, std::string> cgi;
    std::map<unsigned short, std::string>  Error_pages;
    bool autoindex;
    bool is_query_match;

    // Default constructor
    S_LocationMatch()
        : path(""), directory(""), methods(), index_file(""),
          upload_directory(""), cgi(), autoindex(false), is_query_match(false) {}

    // Custom constructor
    S_LocationMatch(std::string p, std::string d, std::vector<std::string> m,
                    std::string i, std::string u, bool a, bool q)
        : path(p), directory(d), methods(m), index_file(i),
          upload_directory(u), cgi(), autoindex(a), is_query_match(q) {}

} t_LocationMatch;

class Client
{
    private:
    
    public:
        long         lastTime;
        Client_state state;
        HttpRequest  httpRequest;
        Server *server;
        route  BestMatch;
        std::string buffer;
        size_t BytesReaded;
        bool       match_found;
        S_LocationMatch LocationMatch;
        ResponseInfos Response;

        void    parse_request(int fd, size_t bytesReaded);
        void    LocationCheck();

        ResponseInfos    GET();
        Client();
        ~Client();

        Client(const Client& other);

        // Copy assignment operator
        Client& operator=(const Client& other);

        static ResponseInfos generateResponse(ResponseType type,  const std::string& path, int statusCode, S_LocationMatch& LocationMatch);
        static std::string getStatusMessage(int statusCode);
};

std::string to_string(int value);