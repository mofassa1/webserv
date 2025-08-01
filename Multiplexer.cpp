
#include "Multiplexer.hpp"

bool Multiplexer::isServerSocket(int fd)
{
    return std::find(fileDiscriptors.begin(), fileDiscriptors.end(), fd) != fileDiscriptors.end();
}

int Multiplexer::create_server_socket(unsigned short currentPort, std::string host)
{
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
            soketOfPort[socketFd] = (Ports[j]);
            struct epoll_event event;

            event.events = EPOLLIN;
            event.data.fd = socketFd;

            if (epoll_ctl(epollFd, EPOLL_CTL_ADD, socketFd, &event) == -1)
                throw std::runtime_error("epoll ctl failed !!!!!");
            this->fileDiscriptors.push_back(socketFd);
            config.GetAllData()[i]->SetServerSocket(socketFd);
        }
    }

    this->run(config);
}
long get_time_ms()
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (ts.tv_sec * 1000L) + (ts.tv_nsec / 1000000L);
}

void Multiplexer::NewClient(confugParser &config, int eventFd)
{

    int clientFd = accept(eventFd, NULL, NULL);
    if (clientFd == -1)
        return;
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = clientFd;
    epoll_ctl(this->EpoleFd, EPOLL_CTL_ADD, clientFd, &event);
    allClients.push_back(clientFd);

    if (clientFd != -1)
    {
        config.newClient(clientFd, eventFd);
        soketOfPort[clientFd] = soketOfPort[eventFd];
        size_t count = config.GetAllData().size();
        for (size_t i = 0; i < count; i++)
        {
            if (config.GetAllData()[i]->isTheSeverSocket(eventFd))
            {
                clientOfServer[clientFd].push_back(config.GetAllData()[i]);
            }
        }
    }
}

void Multiplexer::timeoutCheker(confugParser &config)
{
    for (size_t i = 0; i < allClients.size();)
    {
        int fd = allClients[i];
        std::map<int, Client>::iterator it = client.find(fd);

        if (client[fd].cgiInfos.isRunning == true)
        {
            if (waitpid(client[fd].cgiInfos.childPid, &client[fd].cgiInfos.status, WNOHANG) == 0)
            {
                if (get_time_ms() - client[fd].lastTime > TIMEOUT_MS)
                {
                    client[fd].cgiInfos.isRunning = false;
                    kill(client[fd].cgiInfos.childPid, SIGKILL);
                    client[fd].Response = Client::generateResponse(RESPONSE_ERROR, "", TIMEOUT_504, client[fd]);
                    handelResponse(client[fd], fd, config);
                    close(fd);
                    if (it != client.end())
                    {
                        client.erase(it);
                    }
                    config.removeClient(fd);
                    clientOfServer.erase(fd);
                    allClients.erase(allClients.begin() + i);
                    epoll_ctl(EpoleFd, EPOLL_CTL_DEL, fd, NULL);
                    continue;
                }
            }
            else{
                client[fd].CGI_RESPONSE();
                handelResponse(client[fd], fd, config);
                close(fd);
                if (it != client.end())
                {
                    client.erase(it);
                }
                config.removeClient(fd);
                clientOfServer.erase(fd);
                allClients.erase(allClients.begin() + i);
                epoll_ctl(EpoleFd, EPOLL_CTL_DEL, fd, NULL);
                continue;
            }
        }
        if (get_time_ms() - client[fd].lastTime > TIMEOUT_MS)
        {
            Client curentClient;
            if (it != client.end()){
                curentClient = client[fd];
                curentClient.server_matched = clientOfServer[fd][0]; 
            }
            curentClient.Response = Client::generateResponse(RESPONSE_ERROR, "", TIMEOUT, curentClient);
            handelResponse(curentClient, fd, config);
            close(fd);
            if (it != client.end())
            {
                client.erase(it);
            }
            config.removeClient(fd);
            clientOfServer.erase(fd);
            allClients.erase(allClients.begin() + i);
            epoll_ctl(EpoleFd, EPOLL_CTL_DEL, fd, NULL);
            continue;
        }

        ++i;
    }
}

void Multiplexer::removeClient(confugParser &config, int eventFd)
{
    std::map<int, Client>::iterator iter = client.find(eventFd);
    client.erase(iter);
    epoll_ctl(this->EpoleFd, EPOLL_CTL_DEL, eventFd, NULL);
    config.removeClient(eventFd);
    clientOfServer.erase(eventFd);
    for (size_t index = 0; index < allClients.size(); index++)
    {
        if (allClients[index] == eventFd)
        {
            allClients.erase(allClients.begin() + index);
            break;
        }
    }
    close(eventFd);
}

void Multiplexer::run(confugParser &config)
{
    while (true)
    {
        struct epoll_event events[1024];
        int eventCount = epoll_wait(this->EpoleFd, events, 1024, 200);

        timeoutCheker(config);
        if (eventCount == -1 || eventCount == 0)
        {
            continue;
        }
        for (int i = 0; i < eventCount; i++)
        {
            int eventFd = events[i].data.fd;

            if (isServerSocket(eventFd))
            {
                NewClient(config, eventFd);
            }
            else if (events[i].events & EPOLLIN)
            {
                client[eventFd].lastTime = get_time_ms();
                char buffer[1024];
                ssize_t bytesReaded = read(eventFd, buffer, sizeof(buffer));

                if (bytesReaded <= 0)
                {
                    removeClient(config, eventFd);
                }
                else
                    HandleRequest(eventFd, std::string(buffer, bytesReaded), bytesReaded, config);
            }
            else if (events[i].events & EPOLLOUT)
            {
                client[eventFd].lastTime = get_time_ms();
                if (handelResponse(client[eventFd], eventFd, config))
                    removeClient(config, eventFd);
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
