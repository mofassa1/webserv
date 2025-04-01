#pragma once

#include "confugParser.hpp"

#include <iostream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <algorithm>

#include "Client.hpp"

class Client;

class Multiplexer
{
    private:
        int create_server_socket(unsigned short currentPort, std::string host);
        std::map<int, Client> client;
        std::map<int, std::string> soketOfHost;

    public:
        Multiplexer(/* args */);
        ~Multiplexer();
        void    startMultiplexing(confugParser& config);
        void    run();
        void    NewClient(int eventFd);
        bool    isServerSocket(int fd);
        void    handelRequest(int eventFd);
        void    handelResponse(int eventFd);
        std::vector <int> fileDiscriptors;
        int EpoleFd;
};
