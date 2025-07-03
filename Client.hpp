#pragma once

#include "confugParser.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sstream>
#include <fstream>
#include <stdint.h>
#include <ctime>
#include <iostream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <algorithm>
#include <sys/time.h>
#include <string>
#include <dirent.h>  
#include <sys/types.h> 
#include <sys/wait.h>
#include "HttpRequest.hpp"

#define BUFFERSIZE 1000

enum ResponseType {
    RESPONSE_FILE,
    RESPONSE_ERROR,
    RESPONSE_REDIRECT,
    RESPONSE_DIRECTORY_LISTING,
    RESPONSE_DELETE,
    RESPONSE_CGI_CHECK
};

struct ResponseInfos {
    int status;
    std::string body;
    std::string contentType;
    std::map<std::string, std::string> headers;
};


enum Client_state{
    waiting,
    request_start_line,
    request_headers,
    request_body,
    done
};

typedef struct CGIInfos {
    time_t startTime;
    pid_t childPid;
    std::string inputFileName;
    std::string outputFileName;
    bool isRunning;
    CGIInfos(): startTime(0), childPid(0), inputFileName(""), outputFileName(""), isRunning(false) {}
}t_CGIInfos;

typedef struct S_LocationMatch {
    std::string path;
    std::string directory;
    std::string HOST;
    int         PORT;
    std::vector<std::string> methods;
    std::vector<std::string> index_files;
    std::string upload_directory;
    std::string upload_path;
    std::string redirect_path;
    std::ofstream upload_file;
    std::map<std::string, std::string> cgi;
    bool is_cgi;
    std::string content_type_cgi;
    std::map<unsigned short, std::string>  Error_pages;
    bool autoindex;
    bool is_query_match;
    
S_LocationMatch()
    : path(""), directory(""), HOST(""), PORT(0), methods(), index_files(),
      upload_directory(""), upload_path(""), redirect_path(""), upload_file(),
      cgi(), is_cgi(false), content_type_cgi(""), Error_pages(),
      autoindex(false), is_query_match(false) {}

S_LocationMatch(std::string p, std::string d, std::vector<std::string> m, std::string u, bool a, bool q)
    : path(p), directory(d), HOST(""), PORT(0), methods(m), index_files(),
      upload_directory(u), upload_path(""), redirect_path(""), upload_file(),
      cgi(), is_cgi(false), content_type_cgi(""), Error_pages(),
      autoindex(a), is_query_match(q) {}


} t_LocationMatch;

class Client
{
    private:
    
    public:
        long         lastTime;
        Client_state state;
        HttpRequest  httpRequest;
        Server *server_matched;
        std::vector<Server*> servers;
        route  BestMatch;
        std::string buffer;
        size_t BytesReaded;
        bool       match_found;
        S_LocationMatch LocationMatch;
        ResponseInfos Response;
        t_CGIInfos cgiInfos;

        ResponseInfos executeCGI(const std::string &cgiPath, const std::string &scriptPath);
        bool CGI_RESPONSE();

        void    parse_request(int fd, size_t bytesReaded);
        void    check_HOST();
        void    LocationCheck();

        ResponseInfos    GET();
        ResponseInfos    POST();
        ResponseInfos   DELETE();
        Client();
        ~Client();
        
        Client(const Client& other);
        
        Client& operator=(const Client& other);
        
        ResponseInfos   deleteDir(const std::string& path);
        static ResponseInfos generateResponse(ResponseType type,  const std::string& path, int statusCode, S_LocationMatch& LocationMatch);
        static std::string getStatusMessage(int statusCode);
};

std::string to_string(int value);
bool AreYouNew(int client_sockfd, std::map<int, Client> &clients);
void epoll_change(int &EpoleFd, int &eventFd);
std::string HOST_AND_PORT(std::string HOST, int PORT);
std::string toLower(const std::string &str);
std::string generateUniqueString();
std::string getFileExtension(const std::string &path);
std::string getExtensionFromContentType(const std::string &contentType);
bool isCGI(const std::string &fileExtension, const std::map<std::string, std::string> &cgiMap);
std::string validateUploadDir(const std::string& path);
int countWords(const std::string& input);