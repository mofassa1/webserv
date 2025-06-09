#include "Client.hpp"

Client::Client() : state(waiting), index_route(-1), BytesReaded(0)
{ // Default constructor
    struct timeval tv;
    gettimeofday(&tv, NULL);
    lastTime = (tv.tv_sec * 1000L) + (tv.tv_usec / 1000L);
     
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
    if (!founded1)
        throw 7;

    for (size_t i = 0; i < allowed_methods.size(); i++)
    {
        if (allowed_methods[i] == httpRequest.getMethod())
            founded2 = true;
    }
    if (!founded2)
        throw 7;
}

void Client::parse_request(int fd, size_t _Readed)
{
    switch (state)
    {
    case waiting:
        std::cout << GREEN << "[" << fd << "]" << " WAITING" << COLOR_RESET << std::endl;
        if (!httpRequest.VALID_CRLN_CRLN(buffer))
            break;
        state = request_start_line;
        httpRequest.storethebuffer(buffer);
        /* fall through */
    case request_start_line:
        httpRequest.start_line();
        GetServerMethods(); // 7alit feha gae machakil dyal URL
        std::cout << GREEN << "[" << fd << "]" << "- - - - - - VALID START LINE - - - - - - -" << COLOR_RESET << std::endl;
        state = request_headers;
        /* fall through */
    case request_headers:
        httpRequest.headers();
        std::cout << GREEN << "[" << fd << "]" << "- - - - - - VALID HEADERS - - - - - -" << COLOR_RESET << std::endl;
        if (httpRequest.getMethod() == "POST" && httpRequest.validbody(buffer)){
            std::cout << GREEN << "[" << fd << "]" << "- - - - - - VALID BODY - - - - - - " << COLOR_RESET << std::endl;
            state = request_body;
        }
        else
        {
            state = done;
            break;
        }
        /* fall through */
    case request_body:
        httpRequest.parsebody(buffer, _Readed, BytesReaded);
        if (httpRequest.chunk_done == true)
        {
            state = done;
            std::cout << GREEN << "[" << fd << "]" << "- - - - - - DONE - - - - - -" << COLOR_RESET << std::endl;
        }
        else
            std::cout << GREEN << "[" << fd << "]" << "- - - - - - WAITING FOR MORE DATA - - - - - - -" << COLOR_RESET << std::endl;
        break;
    default:
        break;
    }
}

Client::Client(const Client& other)
{
    lastTime = other.lastTime;
    state = other.state;
    httpRequest = other.httpRequest;
    routes = other.routes;
    index_route = other.index_route;
    allowed_methods = other.allowed_methods;
    server = other.server;  // Shallow copy
    buffer = other.buffer;
    BytesReaded = other.BytesReaded;
}

Client& Client::operator=(const Client& other)
{
    if (this != &other)
    {
        lastTime = other.lastTime;
        state = other.state;
        httpRequest = other.httpRequest;
        routes = other.routes;
        index_route = other.index_route;
        allowed_methods = other.allowed_methods;
        server = other.server;  // Shallow copy
        buffer = other.buffer;
        BytesReaded = other.BytesReaded;
    }
    return *this;
}

