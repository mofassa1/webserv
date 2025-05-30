#include "Client.hpp"

Client::Client() : state(waiting), index_route(-1)
{ // Default constructor
  // std::cout << "Client default constructor called" << std::endl;
}

Client::~Client()
{
    // std::cout << "Client destructor called" << std::endl;
}

void Client::GetServerMethods()
{
    bool founded1 = false;
    bool founded2 = false;
    httpRequest.validstartline();

    for (size_t i = 0; i < server->GetRoute().size(); i++)
    {
        if (server->GetRoute()[i].GetPats()["path:"] == httpRequest.getUrl())
        {
            allowed_methods = server->GetRoute()[i].GetMethods();
            founded1 = true;
            break;
        }
    }
    if(!founded1)
        throw 7;

    for(size_t i = 0; i < allowed_methods.size(); i++){
        if(allowed_methods[i] == httpRequest.getMethod())
            founded2 = true;    
    }
    if(!founded2)
        throw 7;
}

void Client::parse_request(int fd)
{
    std::cout << GREEN << "[" << fd << "]" << " WAITING" << COLOR_RESET << std::endl;
    std::cout << RED << buffer << COLOR_RESET << std::endl;
    switch (state)
    {
    case waiting:
        if (!httpRequest.VALID_CRLN_CRLN(buffer))
            break;
        state = request_start_line;
        httpRequest.storethebuffer(buffer);
        /* fall through */
    case request_start_line:
        std::cout << GREEN << "[" << fd << "]" << COLOR_RESET << std::endl;
        httpRequest.start_line();
        GetServerMethods(); // 7alit feha gae machakil dyal URL
        
        std::cout << GREEN << "- - - - - - VALID START LINE - - - - - - -" << COLOR_RESET << std::endl;
        state = request_headers;
        /* fall through */
    case request_headers:
        std::cout << GREEN << "[" << fd << "]" << COLOR_RESET << std::endl;
        httpRequest.headers();
        state = request_body;
        /* fall through */
    case request_body:
        if (httpRequest.getMethod() == "POST")
        {
            httpRequest.getbody();
            std::cout << GREEN << "get body" << std::endl;
        }
        break;
    default:
        break;
    }
}