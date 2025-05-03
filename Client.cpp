#include "Client.hpp"

Client::Client(): state(waiting) {  // Default constructor
    // std::cout << "Client default constructor called" << std::endl;
}

Client::~Client()
{
    // std::cout << "Client destructor called" << std::endl;
}


void    Client::handle_request(const std::string& buffer){
    (void)buffer;
    switch(state){
        case waiting:
            std::cout << GREEN << "WAITING" << COLOR_RESET << std::endl;
                // if(httpRequest.VALID_CRLN_CRLN(buffer))
                    state = request_start_line;
            break;
        case request_start_line :
            std::cout << GREEN << "request start line " << std::endl;
            state = request_headers;
            break;
        case request_headers:
            std::cout << GREEN << "request headears" << std::endl;
            state = request_body;
            break;
        case request_body: 
            std::cout << GREEN << "request request body" << std::endl;
            state = done;
            break;
        default:
            break;
    }
}