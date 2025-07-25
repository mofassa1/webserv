#pragma once
#include "route.hpp"
#define MaxSize 500


class Server
{
    private:
        std::vector<int> sockets;
        std::vector<int> clientSocets;
        std::string host;
        std::vector<unsigned short> ports;
        size_t client_body_size_limit;
        std::string server_name;
        std::map<unsigned short, std::string> default_error_pages;
        std::vector<route> routs;

    public:
        //////////// Seters //////////
        void SetHost(std::string value);
        void SetPorts(unsigned short value);
        void Setclient_body_size_limit(size_t value);
        void SetServerName(std::string value);
        void SetDefaultERRPages(unsigned short key, std::string value);
        void SetRoute(route Route);

        ////
        void SetServerSocket(int socket);
        void setClientSocket(int socket);
        
        //////////// Geters ////////////
        std::string GetHost(void);
        std::vector<unsigned short> GetPorts(void);
        size_t Getclient_body_size_limit(void);
        std::string GetServerName(void);
        std::map<unsigned short, std::string>  &GetDefaultERRPages(void);
        std::vector<route> GetRoute(void);
        
        ///////////// sokets ////////////
        std::vector<int> GetServerSockets();
        const std::vector <int> getClientSockets() const;
 
        //// extra methods ///////////////

        bool isTheSeverClient(int socket) const;
        bool isTheSeverSocket(int socket) const;

        void removeClient(int socket);
        int fd;

        Server();
        ~Server();

        Server& operator=(const Server& other)
        {
            if (this != &other) // avoid self-assignment
            {
                sockets = other.sockets;
                clientSocets = other.clientSocets;
                host = other.host;
                ports = other.ports;
                client_body_size_limit = other.client_body_size_limit;
                server_name = other.server_name;
                default_error_pages = other.default_error_pages;
                routs = other.routs;
                fd = other.fd;
            }
            return *this;
        }
};

