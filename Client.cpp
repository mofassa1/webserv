#include "Client.hpp"


void    Client::parseBody(std::string buff, int buffetLen) {

    this->body.append(buff, 0, buffetLen);
}

void    Client::parseHeader(std::string buff, int buffetLen) {

    this->header.append(buff, 0, buffetLen);
}

void   Client::GetBuffer(std::string buff, int buffetLen){

    if (!isHeader)
        this->parseBody(buff, buffetLen);
    else
    {
        this->request.append(buff);
        size_t index = this->request.find("\r\n\n\r");
        
        if (index == std::string::npos)
        {
            parseHeader(this->request, buffetLen);
        }
        else
        {
            // std::string headerPart = this->request.substr(0, index);
            // std::string bodyPart   = this->request.substr(index + 4);
            parseHeader(this->request.substr(0, index), index + 1); /// check later
            parseBody(this->request.substr(index + 4), buffetLen - index - 3); // also need to check later
            this->isHeader = false;
        }
    }
}

Client::Client(/* args */)
{
    this->isHeader = true;
}

Client::~Client()
{
}
