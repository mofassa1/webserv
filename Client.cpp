#include "Client.hpp"

Client::Client() : state(waiting), BytesReaded(0)
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

void Client::LocationCheck()
{
    LocationMatch.path = httpRequest.getDecodedPath();
    const std::vector<route>& routes = server->GetRoute();

    size_t best_match_len = 0;
    bool match_found = false;

    for (size_t i = 0; i < routes.size(); ++i)
    {
        std::string route_path = server->GetRoute()[i].GetPats()["path:"];
        if (LocationMatch.path.compare(0, route_path.length(), route_path) == 0 &&
            (route_path.length() > best_match_len))
        {
            BestMatch = routes[i];
            best_match_len = route_path.length();
            match_found = true;
        }
    }

    if (!match_found)
        throw 404;

    LocationMatch.methods = BestMatch.GetMethods();
    bool method_allowed = false;
    for (size_t i = 0; i <  LocationMatch.methods.size(); ++i)
    {
        if (LocationMatch.methods[i] == httpRequest.getMethod())
        {
            method_allowed = true;
            break;
        }
    }

    if (!method_allowed)
        throw 405;
    //if(httpRequest.getMethod() == "POST")
    // LocationMatch.upload_directory = BestMatch.getUpload_directory
    // if upload_directory is empty throw error UNTHORIZED
    // upload PATH 
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

        // GetServerMethods(); // 7alit feha gae machakil dyal URL
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
            state = done;
        break;
    default:
        break;
    }
}


void Client::GET(){
    
}


Client::Client(const Client& other)
{
    lastTime = other.lastTime;
    state = other.state;
    httpRequest = other.httpRequest;
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
        server = other.server;  // Shallow copy
        buffer = other.buffer;
        BytesReaded = other.BytesReaded;
    }
    return *this;
}

