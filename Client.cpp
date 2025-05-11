#include "Client.hpp"

Client::Client(): state(waiting) {  // Default constructor
    // std::cout << "Client default constructor called" << std::endl;
}

Client::~Client()
{
    // std::cout << "Client destructor called" << std::endl;
}


void    Client::handle_request(const std::string& buffer, int fd){
    switch(state){
        case waiting:
            std::cout << GREEN << fd << " WAITING" << COLOR_RESET << std::endl;
            std::cout << RED << buffer << COLOR_RESET << std::endl; 
                if(!httpRequest.VALID_CRLN_CRLN(buffer))
                    break;
                //  state = request_start_line;
                std::cout << GREEN << "GO TO START LINE" << std::endl;
        case request_start_line :
            std::cout << GREEN << "request start line " << std::endl;
            // httpRequest.start_line();
            state = request_headers;
        case request_headers:
            std::cout << GREEN << "request headears" << std::endl;
            state = request_body;
        case request_body: 
            std::cout << GREEN << "request request body" << std::endl;
            state = done;
        default:
            break;
    }
}