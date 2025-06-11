#pragma once

#include "confugParser.hpp"

#include <iostream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <algorithm>
#include <sys/time.h>
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
    
    public:
        long         lastTime;
        Client_state state;
        HttpRequest  httpRequest;
        std::vector<std::string> allowed_methods;
        Server *server;
        route  match;
        std::string buffer;
        size_t BytesReaded;

        void    parse_request(int fd, size_t bytesReaded);
        void    LocationCheck();

        void    GET();
        Client();
        ~Client();

        Client(const Client& other);

        // Copy assignment operator
        Client& operator=(const Client& other);
};
