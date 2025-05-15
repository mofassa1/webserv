#pragma once

#include "confugParser.hpp"

#include <iostream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <algorithm>

#include "HttpRequest.hpp"

#define BUFFERSIZE 1000

enum Client_state{
    waiting,
    request_start_line,
    request_headers,
    request_body,
    done
};

class Client
{
    private:
        HttpRequest httpRequest;
        Client_state state;
        
        public:
        std::vector<std::string> allowed_methods;
        Server *server;
        std::string buffer;
        int BytesReaded;

        void    parse_request(int fd);
        Client();
        ~Client();
};
