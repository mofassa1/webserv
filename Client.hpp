#pragma once

#include "confugParser.hpp"

#include <iostream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <algorithm>

class Client
{
    private:
        // int fd; // i may remove it later
        bool  isHeader;
        std::string request;
        std::string header;
        std::string body;
        std::map<std::string, std::string> HeaderAtrributes;

    public:

        // void   getRequestBuff(std::string buff, int buffetLen);
        // void   parseBuffer(std::string buff, int buffetLen);
        void   GetBuffer(std::string buff, int buffetLen);
        void   parseHeader(std::string buff, int buffetLen);
        void   parseBody(std::string buff, int buffetLen);
        
        Client(/* args */);
        ~Client();
};

