#include "HttpRequest.hpp"


HttpRequest::HttpRequest(): hasContentLength(false), hasTransferEncoding(false), body_received(0),
    reading_chunk_size(true), reading_chunk_data(false), chunk_done(false)
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

void HttpRequest::validstartline()
{
    if (vstart_line.size() != 7)
        throw BAD_REQUEST;
    if(vstart_line[0] != "GET" && vstart_line[0] != "DELETE" && vstart_line[0] != "POST")
        throw BAD_REQUEST;
    if (vstart_line[1] != " ")
        throw BAD_REQUEST;
    if (vstart_line[2][0] != '/')
        throw BAD_REQUEST;
    if (vstart_line[3] != " ")
        throw BAD_REQUEST;
    if (vstart_line[4] != "HTTP/1.1" && vstart_line[4] != "HTTP/1.0")
        throw BAD_REQUEST;
    if (vstart_line[5] != "\r")
        throw BAD_REQUEST;
    if (vstart_line[6] != "\n")
        throw BAD_REQUEST;
    
    tstart_line.method = vstart_line[0];
    tstart_line.url = vstart_line[2];
    tstart_line.version = vstart_line[4];
    parseRequestUri(vstart_line[2]);
}

void HttpRequest::start_line()
{
    if (lines.empty())
        throw std::exception();
    split_line(lines[0], vstart_line);
    validstartline();
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
        throw 1;
    }
    std::string header_name = buffer.substr(0, pos);
    if (header_name.find(" ") != std::string::npos)
        throw 2;
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
        std::cout << it->first << " => " << it->second << std::endl;
        
    }
    std::cout << COLOR_RESET;
}

void HttpRequest::headers()
{
    std::vector<std::string> vheaders;
    if (lines.size() < 2)
        throw 3;
    for (size_t i = 1; i < lines.size(); i++)
    {
        if (lines[i] == "\r\n")
            break;
        split_header(lines[i], vheaders);
        if (!validheader(vheaders))
            throw 5;
        vheaders.clear();
    }
    print_map(mheaders);
}


bool HttpRequest::validbody(const std::string& buffer, size_t maxsize){
    hasContentLength = mheaders.find("Content-Length") != mheaders.end();
    hasTransferEncoding = mheaders.find("Transfer-Encoding") != mheaders.end();
    if (hasContentLength && hasTransferEncoding)
        throw 1;

    if (!hasContentLength && !hasTransferEncoding) 
        throw LENGTH_REQUIRED;
    
    if(hasContentLength) {
        char *end;
        std::string content_length_str = mheaders["Content-Length"];
        content_length_str.erase(0, content_length_str.find_first_not_of(" \t\r\n"));
        content_length_str.erase(content_length_str.find_last_not_of(" \t\r\n") + 1);
        content_length = static_cast<size_t>(strtoul(content_length_str.c_str(), &end, 10));
        if (*end != '\0') // content_length == 0 || supprimed
            throw BAD_REQUEST;
        if (content_length > maxsize)
            throw BAD_REQUEST; // Content-Length exceeds max size
    }
    if (hasTransferEncoding) {
        if (mheaders["Transfer-Encoding"] != "chunked\r\n")
            throw BAD_REQUEST; // Only chunked transfer encoding is supported
    }
    initBodyReadIndex(buffer);
    return true;
}

void HttpRequest::initBodyReadIndex(const std::string& buffer) {
    const std::string delimiter = "\r\n\r\n";
    size_t pos = buffer.find(delimiter);

    // Body starts right after "\r\n\r\n"
    _Read_index_body = pos + delimiter.length();
}

void HttpRequest::checkBodyCompletionOnEOF()
{
    if (hasContentLength && !chunk_done)
        throw 2;

    if (hasTransferEncoding && !chunk_done)
        throw 3;
}

void HttpRequest::contentLength(const std::string &buffer, size_t totalbytesReaded, std::ofstream& upload_file, size_t EndofFile)
{
    size_t remaining = content_length - body_received;
    size_t available = totalbytesReaded - _Read_index_body;
    size_t to_read = std::min(remaining, available);

    for (size_t i = 0; i < to_read; ++i) {
        char ch = buffer[_Read_index_body];
        body += ch;
        upload_file.write(&ch, sizeof(ch));
        _Read_index_body++;
        body_received++;
    }

    if (body_received == content_length)
        chunk_done = true;
}


void HttpRequest::TransferEncoding(const std::string &buffer, size_t totalbytesReaded, std::ofstream& upload_file)
{
   (void)totalbytesReaded;
    while (_Read_index_body < buffer.size() && !chunk_done)
    {
        if (reading_chunk_size)
        {
            while (_Read_index_body < buffer.size())
            {
                char c = buffer[_Read_index_body++];
                if (c == '\r')
                    continue;
                if (c == '\n')
                    break;
                chunk_size_line += c;
            }
            if (!chunk_size_line.empty() && buffer[_Read_index_body - 1] == '\n')
            {
                char *end;
                current_chunk_size = static_cast<size_t>(strtoul(chunk_size_line.c_str(), &end, 16));
                if (*end != '\0')
                    throw BAD_REQUEST;
                chunk_size_line.clear();
                if (current_chunk_size == 0)
                {
                    chunk_done = true;
                    reading_chunk_size = false;
                    reading_chunk_data = false;
                    return;
                }

                reading_chunk_size = false;
                reading_chunk_data = true;
            }
            else
            {
                return;
            }
        }
        if (reading_chunk_data)
        {
            size_t remaining_in_buffer = buffer.size() - _Read_index_body;
            size_t to_copy = std::min(current_chunk_size, remaining_in_buffer);

            body.append(buffer, _Read_index_body, to_copy);
            upload_file.write(buffer.data() + _Read_index_body, to_copy);
            _Read_index_body += to_copy;
            current_chunk_size -= to_copy;

            if (current_chunk_size == 0)
            {
                reading_chunk_data = false;
                reading_chunk_size = true;

                if (_Read_index_body + 1 < buffer.size() &&
                    buffer[_Read_index_body] == '\r' &&
                    buffer[_Read_index_body + 1] == '\n')
                {
                    _Read_index_body += 2;
                }
                else if (_Read_index_body < buffer.size() && buffer[_Read_index_body] == '\n')
                {
                    _Read_index_body += 1;
                }
                else
                {
                    return; 
                }
            }
            else
            {
                return;
            }
        }
    }
}

void HttpRequest::parsebody(const std::string& buffer, size_t bytesReaded, size_t totalbytesReaded, std::ofstream& upload_file)
{  
    if (hasContentLength)
        contentLength(buffer, totalbytesReaded, upload_file, bytesReaded); 
    if (hasTransferEncoding)
        TransferEncoding(buffer, totalbytesReaded, upload_file);
}

void HttpRequest::parseRequestUri(const std::string &Uri){
    std::string uri = Uri;

    size_t scheme_pos = uri.find("://");
    if (scheme_pos != std::string::npos) {
        size_t path_start = uri.find('/', scheme_pos + 3);
        if (path_start != std::string::npos)
            uri = uri.substr(path_start);
        else
            uri = "/";
    }

    if (uri.empty() || uri[0] != '/')
        throw BAD_REQUEST;

    if (uri.length() > 2048)
        throw 500;

    if (isBadUri(uri) || isBadUriTraversal(uri))
        throw BAD_REQUEST;

    std::string path;
    size_t qmark = uri.find('?');
    if (qmark != std::string::npos) {
        path = uri.substr(0, qmark);
        std::string query = uri.substr(qmark + 1);
        parseParams(query, query_params);
    } else {
        path = uri;
    }
    decoded_path = decodePercentEncoding(path);
}

bool HttpRequest::VALID_CRLN_CRLN(const std::string &buffer)
{
    std::size_t header_end = buffer.find("\r\n\r\n");

    if (header_end != std::string::npos)
        return true;
    return false;
}

std::string HttpRequest::getMethod() const
{
    return tstart_line.method;
}

std::string HttpRequest::getDecodedPath() const{
    return decoded_path;
}


std::string HttpRequest::getVersion() const
{
    return tstart_line.version;
}

std::string HttpRequest::GetHeaderContent(std::string HEADER) {
    std::map<std::string, std::string>::const_iterator it = mheaders.find(HEADER);
    if (it != mheaders.end())
        return it->second;
    return "";
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