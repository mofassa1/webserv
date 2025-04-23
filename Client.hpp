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

class Client
{
    private:
        HttpRequest httpRequest;

    public:
        std::string buffer;
        int BytesReaded;
        Client();
        ~Client();
};
