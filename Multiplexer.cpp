
#include "Multiplexer.hpp"

bool Multiplexer::isServerSocket(int fd)
{
    return std::find(fileDiscriptors.begin(), fileDiscriptors.end(), fd) != fileDiscriptors.end();
}

int Multiplexer::create_server_socket(unsigned short currentPort, std::string host)
{

    //// I need first to check is already have a server with the same host , Port ///////

    static std::vector<std::pair<unsigned short, std::string> > binded;
    static std::map<std::pair<unsigned short, std::string>, int> socketServer;

    bool found = false;
    std::pair<unsigned short, std::string> key_to_find = std::make_pair(currentPort, host);
    for (std::vector<std::pair<unsigned short, std::string> >::iterator it = binded.begin(); it != binded.end(); ++it)
    {
        if (it->first == key_to_find.first && it->second == key_to_find.second)
        {
            found = true;
            break;
        }
    }

    if (found)
    {
        return socketServer[key_to_find] * -1;
    }

    int Socket = socket(AF_INET, SOCK_STREAM, 0);
    if (Socket == -1)
        throw std::runtime_error("socket failed !!!!!!!");

    struct sockaddr_in address;
    memset(address.sin_zero, '\0', sizeof address.sin_zero);
    address.sin_family = AF_INET;
    if (inet_pton(AF_INET, host.c_str(), &address.sin_addr) <= 0)
        throw std::runtime_error("inet_pton");

    address.sin_port = htons(currentPort);
    const int enable = 1;
    if (setsockopt(Socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        throw std::runtime_error("setsockopt(SO_REUSEADDR) failed");
    if (bind(Socket, (struct sockaddr *)&address, sizeof(address)) < 0)
        throw std::runtime_error("bind failed !!!!!!!!!!");

    if (listen(Socket, SOMAXCONN) < 0)
        throw std::runtime_error("listen failed !!!!!!!!!!");

    binded.push_back(key_to_find);
    socketServer[key_to_find] = Socket;

    return Socket;
}

void Multiplexer::startMultiplexing(confugParser &config)
{

    int epollFd;

    if ((epollFd = epoll_create(1)) == -1)
        throw std::runtime_error("epoll_create failed !!!!");
    this->EpoleFd = epollFd;

    size_t serversCount = config.GetAllData().size();
    for (size_t i = 0; i < serversCount; i++)
    {

        std::vector<unsigned short> Ports = config.GetAllData()[i]->GetPorts();
        // std::vector<int> ServerSocket = config.GetAllData()[i]->GetServerSockets();
        std::string host = config.GetAllData()[i]->GetHost();
        size_t portsCount = Ports.size();
        for (size_t j = 0; j < portsCount; j++)
        {
            int socketFd = this->create_server_socket(Ports[j], host);
            if (socketFd < 0)
            {
                config.GetAllData()[i]->SetServerSocket(socketFd * -1);
                continue;
            }
            struct epoll_event event;

            event.events = EPOLLIN;
            event.data.fd = socketFd;

            if (epoll_ctl(epollFd, EPOLL_CTL_ADD, socketFd, &event) == -1)
                throw std::runtime_error("epoll ctl failed !!!!!");
            this->fileDiscriptors.push_back(socketFd);
            config.GetAllData()[i]->SetServerSocket(socketFd);
        }
    }

    ///////
    this->run(config);
}

int Multiplexer::NewClient(int eventFd)
{

    int clientFd = accept(eventFd, NULL, NULL);
    if (clientFd == -1)
    {
        std::cerr << "accept failed" << std::endl;
        return -1;
    }

    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = clientFd;
    epoll_ctl(this->EpoleFd, EPOLL_CTL_ADD, clientFd, &event);
    return clientFd;
}

bool AreYouNew(int client_sockfd, std::map<int, Client> &clients) {
        return clients.find(client_sockfd) == clients.end();
}

void Multiplexer::handelRequest(int eventFd, std::string buffer, size_t bytesReaded)
{
    if (AreYouNew(eventFd, client)) {
        client[eventFd] = Client();  // Only now, create a new client
    }
    Client& c = client[eventFd];

    c.buffer += buffer;
    c.BytesReaded += bytesReaded;
    c.server = clientOfServer[eventFd];

    c.handle_request(c.buffer);

    /////////////////////////////////
    // struct epoll_event event;
    // event.events = EPOLLOUT | EPOLLET;
    // event.data.fd = eventFd;

    // if (epoll_ctl(this->EpoleFd, EPOLL_CTL_MOD, eventFd, &event) == -1)
    // {
    //     std::cerr << "epoll_ctl failed to modify event" << std::endl;
    // }
}

void Multiplexer::handelResponse(int eventFd, confugParser &confug)
{

    if (eventFd == -1)
    {
        std::cerr << "fd is invalid, connection closed?" << std::endl;
        return;
    }

    // std::cout << "handelResponse of fd " << eventFd << std::endl;
    std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, world!";

    ssize_t bytesSent = send(eventFd, response.c_str(), response.size(), MSG_NOSIGNAL);

    if (bytesSent == -1)
    {
        std::cerr << "send failed for fd: " << eventFd << std::endl;
        close(eventFd);
        epoll_ctl(this->EpoleFd, EPOLL_CTL_DEL, eventFd, NULL);
        confug.removeClient(eventFd);
        return;
    }

    std::cerr << "send succesfull for fd: " << eventFd << std::endl;
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = eventFd;

    if (epoll_ctl(this->EpoleFd, EPOLL_CTL_MOD, eventFd, &event) == -1)
    {
        std::cerr << "epoll_ctl failed to modify event" << std::endl;
    }
}

void Multiplexer::run(confugParser &config)
{

    while (true)
    {

        struct epoll_event events[1024];
        int eventCount = epoll_wait(this->EpoleFd, events, 1024, -1);
        if (eventCount == -1)
        {
            std::cerr << "epoll_wait failed" << std::endl;
            continue;
        }
        else if (eventCount == 0)
            continue;

        for (int i = 0; i < eventCount; i++)
        {
            int eventFd = events[i].data.fd;

            // If it is a server socket we need to accept new client
            if (isServerSocket(eventFd))
            {
                int clientSocket = NewClient(eventFd);
                if (clientSocket != -1)
                {
                    config.newClient(clientSocket, eventFd);
                    size_t count = config.GetAllData().size();
                    for (size_t i = 0; i < count; i++)
                    {
                        if (config.GetAllData()[i]->isTheSeverSocket(eventFd))
                            clientOfServer[clientSocket] = config.GetAllData()[i];
                    }
                }
            }
            // Check if the event is for reading
            else if (events[i].events & EPOLLIN)
            {
                char buffer[1024];
                size_t bytesReaded = read(eventFd, buffer, 10);

                // if the client disconeect or other issue
                if (bytesReaded <= 0)
                {
                    close(eventFd);
                    epoll_ctl(this->EpoleFd, EPOLL_CTL_DEL, eventFd, NULL);
                    config.removeClient(eventFd);
                    clientOfServer.erase(eventFd);
                }
                else
                {
                    buffer[bytesReaded] = '\0';
                    handelRequest(eventFd, buffer, bytesReaded);
                }
            }
            // Check if the event is for writting
            else if (events[i].events & EPOLLOUT)
            {
                handelResponse(eventFd, config);
                close(eventFd);
                epoll_ctl(this->EpoleFd, EPOLL_CTL_DEL, eventFd, NULL);
                config.removeClient(eventFd);
                clientOfServer.erase(eventFd);
            }
        }
    }
}

Multiplexer::Multiplexer(/* args */)
{
}

Multiplexer::~Multiplexer()
{
}
