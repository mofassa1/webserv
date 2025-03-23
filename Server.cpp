#include "Server.hpp"

/////////// Seters ////////////

void Server::SetHost(std::string value)
{
    if (!this->host.empty())
        throw std::runtime_error("host already exist");
    this->host = value;
}

void Server::SetPorts(unsigned short value){
    this->ports.push_back(value);
}
void Server::Setclient_body_size_limit(size_t value){
    if (this->client_body_size_limit != 0)
        throw std::runtime_error("client_body_size_limit exist");
    this->client_body_size_limit = value;
}

void Server::SetServerName(std::string value){
    if (this->server_name != "")
        throw std::runtime_error("server already named");
    this->server_name = value;
}

void Server::SetDefaultERRPages(unsigned short key, std::string value){
    this->default_error_pages[key] = value;
}

void Server::SetRoute(route Route){
    this->routs.push_back(Route);
}

void Server::SetServerSocket(int socket)
{
    this->sockets.push_back(socket);
}

/// ////// Geters ///////////////

std::string Server::GetHost(void){
    return this->host;
}
std::vector<unsigned short> Server::GetPorts(void){
    return this->ports;
}
size_t Server::Getclient_body_size_limit(void){
    return this->client_body_size_limit;
}
std::string Server::GetServerName(void){
    return this->server_name;
}
std::map<unsigned short, std::string>  Server::GetDefaultERRPages(void){
    return this->default_error_pages;
}
std::vector<route> Server::GetRoute(void){
    return this->routs;
}

std::vector<int> Server::GetServerSockets()
{
    return this->sockets;
}


/////////// constructors ////////

Server::Server() {
    this->host = "";
    this->client_body_size_limit = 0;
    this->server_name = "";
}

Server::~Server() {

    std::cout << "server distructor called !!!" << std::endl;
}
