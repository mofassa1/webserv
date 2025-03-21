#pragma once
#include "route.hpp"
#define MaxSize 50000

////// start of server class ////////////////////////////
class Server
{
    private:
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

        //////////// Geters ////////////
        std::string GetHost(void);
        std::vector<unsigned short> GetPorts(void);
        size_t Getclient_body_size_limit(void);
        std::string GetServerName(void);
        std::map<unsigned short, std::string>  GetDefaultERRPages(void);
        std::vector<route> GetRoute(void);
        
        ///////////// sokets ////////////

        // std::vector <int> 
        int fd;

        Server();
        ~Server();
};