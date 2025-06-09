#pragma once

#include "confugParser.hpp"
///
#include <fcntl.h>
///
#include <iostream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <algorithm>
#include <csignal>
#include "Client.hpp"
#include <sys/time.h>

#define BUFFERSIZE 1000
#define TIMEOUT    100
class Multiplexer
{
    private:
        int create_server_socket(unsigned short currentPort, std::string host);
        std::map<int, Client> client;
        std::map<int, std::string> soketOfHost;
        std::map<int, Server*> clientOfServer;
        std::vector<int> allClients; 

    public:
        Multiplexer(/* args */);
        ~Multiplexer();
        void    startMultiplexing(confugParser& config);
        void    run(confugParser& config);
        int     NewClient(int eventFd);
        bool    isServerSocket(int fd);
        void    handelRequest(int eventFd, std::string buffer, size_t bytesReaded, confugParser &config);
        void    handelResponse(int eventFd, confugParser &confug);
        /// //// signal handeler /////////

        std::vector <int> fileDiscriptors;
        int EpoleFd;
};

