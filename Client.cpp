#include "Client.hpp"

Client::Client() : clientFd(-1) {  // Default constructor
    // std::cout << "Client default constructor called" << std::endl;
}

Client::Client(int _eventFd, int EpoleFd) : clientFd(_eventFd) {  // Use _eventFd here
    epoll.events = EPOLLIN | EPOLLET;  // Set epoll events
    epoll.data.fd = clientFd;  // Associate client fd with epoll
    epoll_ctl(EpoleFd, EPOLL_CTL_ADD, clientFd, &epoll);  // Register with epoll
    std::cout << GREEN << "Client created" << COLOR_RESET << std::endl;
}

Client::~Client()
{
    // std::cout << "Client destructor called" << std::endl;
}

void Client::Request()
{
    char buffer[1024];
    size_t bytesReaded = read(clientFd, buffer, BUFFERSIZE);

    
    std::cout << CYAN;
    std::cout << "-----> bytesReaded: " << bytesReaded << std::endl;
    std::cout << "-----> clientFd: " << clientFd <<  std::endl;
    std::cout << "-> buffer <- " << COLOR_RESET << std::endl 
               << MAGENTA << buffer << COLOR_RESET << std::endl;

    // if the client disconeect or other issue 
    if (bytesReaded <= 0){
        close(clientFd);
        // epoll_ctl(this->EpoleFd, EPOLL_CTL_DEL, eventFd, NULL);
    }
    else
    {
        buffer[bytesReaded] = '\0';
        httpRequest.parseRequest(buffer);
    }
}