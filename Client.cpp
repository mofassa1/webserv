#include "Client.hpp"

void    Client::check_HOST(){
    std::string host_value = httpRequest.GetHost();
    if(host_value.empty())
        throw BAD_REQUEST;
    bool check = false;

    for(int i = 0; i < servers.size(); i++){
        if(servers[i]->GetHost() == host_value){
            server_matched = servers[i];
            check = true;
            break;
        }
        if(HOST_AND_PORT(servers[i]->GetHost(), LocationMatch.PORT) == host_value){
            server_matched = servers[i];
            check = true;
            break;
        }
        if(servers[i]->GetServerName() == host_value){
            server_matched = servers[i];
            check = true;
            break;
        }
        if(HOST_AND_PORT(servers[i]->GetServerName(), LocationMatch.PORT) == host_value){
            server_matched = servers[i];
            check = true;
            break;
        }
    }
    if(!check)
        server_matched = servers[0];
}

void Client::LocationCheck()
{
    check_HOST();
    LocationMatch.path = httpRequest.getDecodedPath();
    const std::vector<route> &routes = server_matched->GetRoute();

    size_t best_match_len = 0;
    for (size_t i = 0; i < routes.size(); ++i)
    {
        std::string route_path = server_matched->GetRoute()[i].GetPats()["path:"];
        size_t len = route_path.length();

        if (LocationMatch.path.compare(0, len, route_path) == 0)
        {
            if (LocationMatch.path.length() == len || LocationMatch.path[len] == '/')
            {
                if (len > best_match_len)
                {
                    BestMatch = routes[i];
                    best_match_len = len;
                    match_found = true;
                }
            }
        }
    }
    if (!match_found)
    {
        for(int i = 0; i < routes.size(); i++)
        {
            if(server_matched->GetRoute()[i].GetPats()["path:"] == "/"){
                BestMatch = routes[i];
                match_found = true;
            }
        }
        if(!match_found)
            throw NOT_FOUND;
    }
    LocationMatch.methods = BestMatch.GetMethods();
    bool method_allowed = false;
    for (size_t i = 0; i < LocationMatch.methods.size(); ++i)
    {
        if (LocationMatch.methods[i] == httpRequest.getMethod())
        {
            method_allowed = true;
            break;
        }
    }
    if (!method_allowed)
        throw NOT_ALLOWED;
    LocationMatch.directory = BestMatch.GetPats()["directory:"];
    LocationMatch.autoindex = BestMatch.GetAutoIndex();
    LocationMatch.index_file = BestMatch.GetPats()["index_file:"];
    LocationMatch.cgi = BestMatch.GetCGI();
    HttpRequest::print_map(LocationMatch.cgi);
    LocationMatch.Error_pages = server_matched->GetDefaultERRPages();
    LocationMatch.redirect_path = BestMatch.GetPats()["redirect:"];
    LocationMatch.path = (LocationMatch.path == "/") ? "" : LocationMatch.path;
    if (httpRequest.getMethod() == "POST")
    {
        // GET FINAL URL
        LocationMatch.upload_directory = BestMatch.GetPats()["upload_directory:"];
        if (LocationMatch.upload_directory.empty())
            throw NOT_FOUND;
        std::string content_typee = httpRequest.GetHeaderContent("content-type");
        if(content_typee.empty())
        {
            content_typee = "application/octet-stream";
        }
        if(content_typee == "" || content_typee == ""){
            LocationMatch.is_cgi = true;
            LocationMatch.content_type_cgi = content_typee;
        }
        std::string file_extension = getExtensionFromContentType(content_typee);
        LocationMatch.upload_path = LocationMatch.directory + LocationMatch.path + "/" + LocationMatch.upload_directory + "/" + generateUniqueString() + file_extension;
        //std::cout << "upload_path: " << LocationMatch.upload_path << std::endl;
        LocationMatch.upload_file.open(LocationMatch.upload_path.c_str(), std::ios::out | std::ios::binary);

        // if (!LocationMatch.upload_file.is_open()) // GO BACK
        //     throw NOT_FOUND;
    }
}

void Client::parse_request(int fd, size_t _Readed)
{
    switch (state)
    {
    case waiting:
        //std::cout << GREEN << "[" << fd << "]" << " WAITING" << COLOR_RESET << std::endl;
        if (!httpRequest.VALID_CRLN_CRLN(buffer))
            break;
        //std::cout << MAGENTA << buffer << std::endl;
        state = request_start_line;
        httpRequest.storethebuffer(buffer);
        /* fall through */
    case request_start_line:
        httpRequest.start_line();
        //std::cout << GREEN << "[" << fd << "]" << "- - - - - - VALID START LINE - - - - - - -" << COLOR_RESET << std::endl;
        state = request_headers;
        /* fall through */
    case request_headers:
        httpRequest.headers();
        LocationCheck();
        //std::cout << GREEN << "[" << fd << "]" << "- - - - - - VALID HEADERS - - - - - -" << COLOR_RESET << std::endl;
        if (httpRequest.getMethod() == "POST" && httpRequest.validbody(buffer, server_matched->Getclient_body_size_limit()))
        {
            //std::cout << GREEN << "[" << fd << "]" << "- - - - - - VALID BODY - - - - - - " << COLOR_RESET << std::endl;
            state = request_body;
        }
        else
        {
            state = done;
            break;
        }
        /* fall through */
    case request_body:
        httpRequest.parsebody(buffer, _Readed, BytesReaded, LocationMatch.upload_file);
        if (httpRequest.chunk_done == true)
            state = done;
        break;
    default:
        break;
    }
}


Client::Client() : state(waiting), BytesReaded(0), match_found(false)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    lastTime = (tv.tv_sec * 1000L) + (tv.tv_usec / 1000L);
}

Client::~Client(){
}

Client::Client(const Client &other)
{
    lastTime = other.lastTime;
    state = other.state;
    httpRequest = other.httpRequest;
    buffer = other.buffer;
    BytesReaded = other.BytesReaded;
}

Client &Client::operator=(const Client &other)
{
    if (this != &other)
    {
        lastTime = other.lastTime;
        state = other.state;
        httpRequest = other.httpRequest;
        buffer = other.buffer;
        BytesReaded = other.BytesReaded;
    }
    return *this;
}
