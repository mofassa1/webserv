#pragma once

#include <iostream>
#include <sstream>
#include <vector>
#include <cctype>
#include <map>

#define BLACK "\033[30m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define WHITE "\033[37m"

#define COLOR_RESET "\033[0m"


#define BAD_REQUEST 400
#define LENGTH_REQUIRED 411

typedef struct s_start_line {
    std::string method;
    std::string url;
    std::string version;
} t_start_line;

class HttpRequest {
private:
    std::vector<std::string> lines;
    t_start_line tstart_line;
    std::map<std::string, std::string> mheaders;
    
public:
    HttpRequest();
    ~HttpRequest();

    // Parsing functions
    
    bool    VALID_CRLN_CRLN(const std::string& buffer);
    void parseRequest(const std::string &buffer);
    void storethebuffer(const std::string &buffer);
    void start_line(std::vector<std::string>& allowed_methods);
    void    headers();
    void    getbody();

    bool validstartline(std::vector<std::string> &vstart_line, std::vector<std::string>& allowed_methods);
    bool validheader(const std::vector<std::string> &vheader);
    // bool validbody(const std::string &line);

    static void    split_header(const std::string &buffer, std::vector<std::string> &words);
    static void    split_line(const std::string &buffer, std::vector<std::string>& words);

    // print 
    static void print_vector(std::vector<std::string> &vec);
    static void print_map(const std::map<std::string, std::string> &m);

    // Getters
    std::string getMethod() const;
    std::string getUrl() const;
    std::string getVersion() const;

    bool hasContentLength;
    bool hasTransferEncoding;
};
