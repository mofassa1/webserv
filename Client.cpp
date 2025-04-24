#include "Client.hpp"

Client::Client(): state(waiting) {  // Default constructor
    // std::cout << "Client default constructor called" << std::endl;
}

Client::~Client()
{
    // std::cout << "Client destructor called" << std::endl;
}


void    Client::handle_request(){
}