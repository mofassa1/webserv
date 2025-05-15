#include "HttpRequest.hpp"

HttpRequest::HttpRequest()
{
}

HttpRequest::~HttpRequest()
{
}

void HttpRequest::storethebuffer(const std::string &buffer)
{
    size_t start = 0;

    if (buffer.empty())
        throw std::exception();

    for (size_t i = 0; i < buffer.size(); i++)
    {
        if (buffer[i] == '\n')
        {
            lines.push_back(buffer.substr(start, i - start + 1));
            start = i + 1;
        }
    }
    if (start < buffer.size())
    {
        lines.push_back(buffer.substr(start));
    }
}

void HttpRequest::split_line(const std::string &buffer, std::vector<std::string> &words)
{
    std::string word_to_push;
    size_t i = 0;

    while (i < buffer.size())
    {
        if (std::isspace(buffer[i]) || buffer[i] == '\r' || buffer[i] == '\n')
        {
            words.push_back(std::string(1, buffer[i]));
            i++;
        }
        else
        {
            word_to_push.clear();
            while (buffer[i] && buffer[i] != ' ' && buffer[i] != '\r' && buffer[i] != '\n')
            {
                word_to_push += buffer[i];
                i++;
            }
            words.push_back(word_to_push);
        }
    }
}

bool HttpRequest::validstartline(std::vector<std::string> &vstart_line)
{
    if (vstart_line.size() != 7)
        return false;
    if (vstart_line[0] != "GET" && vstart_line[0] != "POST" && vstart_line[0] != "DELETE")
        return false;
    if (vstart_line[1] != " ")
        return false;
    if (vstart_line[2][0] != '/')
        return false;
    if (vstart_line[3] != " ")
        return false;
    if (vstart_line[4] != "HTTP/1.1" && vstart_line[4] != "HTTP/1.0")
        return false;
    if (vstart_line[5] != "\r")
        return false;
    if (vstart_line[6] != "\n")
        return false;
    return true;
}

void HttpRequest::start_line()
{
    if (lines.empty())
        throw std::exception();
    std::vector<std::string> vstart_line;
    split_line(lines[0], vstart_line);
    if (!validstartline(vstart_line))
        throw BAD_REQUEST;
    tstart_line.method = vstart_line[0];
    tstart_line.url = vstart_line[2];
    tstart_line.version = vstart_line[4];
    std::cout << GREEN << "- - - - - - VALID START LINE - - - - - - -" << COLOR_RESET << std::endl;
}

bool validheadername(const std::string &name)
{
    if (name.empty())
        return false;

    for (size_t i = 0; i < name.size(); i++)
    {
        if (!std::isalnum(name[i]) && name[i] != '-')
            return false;
    }
    return true;
}

bool HttpRequest::validheader(const std::vector<std::string> &vheader)
{
    if (vheader.size() != 2)
        return false;
    if (!validheadername(vheader[0]))
        return false;
    mheaders[vheader[0]] = vheader[1];
    return true;
}

void HttpRequest::split_header(const std::string &buffer, std::vector<std::string> &words)
{
    size_t pos = buffer.find(":");

    if (pos == std::string::npos || pos == 0)
    {
        words.clear();
        throw BAD_REQUEST;
    }
    std::string header_name = buffer.substr(0, pos);
    if (header_name.find(" ") != std::string::npos)
        throw BAD_REQUEST;
    std::string header_value = buffer.substr(pos + 1);
    if (!header_value.empty() && header_value[0] == ' ')
        header_value.erase(0, 1);
    words.clear();
    words.push_back(header_name);
    words.push_back(header_value);
}

void HttpRequest::print_map(const std::map<std::string, std::string> &m)
{
    std::cout << YELLOW;
    for (std::map<std::string, std::string>::const_iterator it = m.begin(); it != m.end(); ++it)
    {
        std::cout << it->first << " => " << it->second;
    }
    std::cout << COLOR_RESET;
}

void HttpRequest::headers()
{
    std::vector<std::string> vheaders;
    if (lines.size() < 2)
        throw BAD_REQUEST;
    for (size_t i = 1; i < lines.size(); i++)
    {
        if (lines[i] == "\r\n")
            break;
        split_header(lines[i], vheaders);
        if (!validheader(vheaders))
            throw BAD_REQUEST;
        vheaders.clear();
    }
    std::cout << GREEN << "- - - - - - VALID HEADERS - - - - - -" << COLOR_RESET << std::endl;
    // print_map(mheaders);
}

void HttpRequest::getbody()
{   
    if (mheaders.find("Content-Length") != mheaders.end()) {
        std::string value = mheaders["Content-Length"];
        // Do something with the value
    } 
    else if()
    else {
        // Key does not exist
    }
}

bool HttpRequest::VALID_CRLN_CRLN(const std::string &buffer)
{
    std::size_t header_end = buffer.find("\r\n\r\n");

    if (header_end != std::string::npos)
        return true;
    return false;
}

void HttpRequest::parseRequest(const std::string &buffer)
{
    storethebuffer(buffer);
    start_line();
    headers();
    getbody();
}

std::string HttpRequest::getMethod() const
{
    return tstart_line.method;
}

std::string HttpRequest::getUrl() const
{
    return tstart_line.url;
}

std::string HttpRequest::getVersion() const
{
    return tstart_line.version;
}

void HttpRequest::print_vector(std::vector<std::string> &vec)
{
    std::cout << YELLOW;
    for (size_t i = 0; i < vec.size(); i++)
    {
        std::cout << vec[i];
    }
    std::cout << COLOR_RESET;
}