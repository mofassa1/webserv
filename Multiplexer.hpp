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

#define CGI_TIMEOUT_MS 5000
#define TIMEOUT_MS 10000

class Multiplexer
{
    private:
        int create_server_socket(unsigned short currentPort, std::string host);
        std::map<int, Client> client;
        std::map<int, std::string> soketOfHost;
        std::map<int, int> soketOfPort;
        std::map<int, std::vector<Server*>> clientOfServer;
        std::vector<int> allClients;

    public:
        Multiplexer(/* args */);
        ~Multiplexer();
        void    startMultiplexing(confugParser& config);
        void    run(confugParser& config);
        void     NewClient(confugParser &config, int eventFd);
        bool    isServerSocket(int fd);
        void    HandleRequest(int eventFd, const std::string& buffer, size_t bytesReaded, confugParser &config);
        bool    handelResponse(Client& client, int eventfd, confugParser &confug);
        void timeoutCheker(confugParser &config);
        void removeClient(confugParser &config, int eventFd);
        /// //// signal handeler /////////

        std::vector <int> fileDiscriptors;
        int EpoleFd;
};