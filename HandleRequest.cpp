#include "Multiplexer.hpp"

void Multiplexer::HandleRequest(int eventFd, const std::string& buffer, size_t bytesReaded, confugParser &config)
{
    try
    {
        if (AreYouNew(eventFd, client))
        {
            client[eventFd] = Client();
        }
        Client &c = client[eventFd];
        c.buffer += buffer;
        c.BytesReaded += bytesReaded;
        c.servers = clientOfServer[eventFd];
        c.LocationMatch.PORT = soketOfPort[eventFd];
        c.parse_request(eventFd, bytesReaded);
        if (c.state == done)
        {
            if (c.httpRequest.getMethod() == "GET")
                c.Response = c.GET();
            if (c.httpRequest.getMethod() == "POST")
                c.Response = c.POST();
            if (c.httpRequest.getMethod() == "DELETE")
                c.Response = c.DELETE();
            
            epoll_change(this->EpoleFd, eventFd);
        }
    }
    catch (int error)
    {
        if(client[eventFd].cgiInfos.isRunning)
            client[eventFd].cgiInfos.isRunning = false;
        client[eventFd].Response = Client::generateResponse(RESPONSE_ERROR, "", error, client[eventFd]);
        epoll_change(this->EpoleFd, eventFd);
    }
    catch (std::exception &e)
    {
        epoll_ctl(this->EpoleFd, EPOLL_CTL_DEL, eventFd, NULL);
        config.removeClient(eventFd);
        clientOfServer.erase(eventFd);
        client.erase(eventFd);
        bool removed = false;
        for (size_t index = 0; index < allClients.size(); index++)
        {
            if (allClients[index] == eventFd)
            {
                removed = true;
                
                allClients.erase(allClients.begin() + index);
                break;
            }
        }
        close(eventFd);
    }
}
