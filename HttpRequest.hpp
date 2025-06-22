#pragma once

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <cctype>
#include <map>
#include <string>
#include <cstdlib>

#define BLACK "\033[30m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define WHITE "\033[37m"

#define COLOR_RESET "\033[0m"

#define OK 200
#define BAD_REQUEST 400
#define UNAUTHORIZED 401
#define FORBIDDEN 403
#define NOT_FOUND 404
#define NOT_ALLOWED 405
#define TIMEOUT 408
#define INTERNAL 500
#define NOT_IMPLEMENTED 501
#define BADGATE 502
#define SERVICE_UNVAILABLE 503
#define UNKNOWN 0
#define LENGTH_REQUIRED 411
#define NO_CONTENT 204
#define ACCEPTED 202


typedef struct s_start_line
{
    std::string method;
    std::string url;
    std::string version;
} t_start_line;

class HttpRequest
{
private:
    std::vector<std::string> lines;
    t_start_line tstart_line;
    std::map<std::string, std::string> mheaders;
    std::vector<std::string> vstart_line;
    std::string body;
    std::string decoded_path;
    std::map<std::string, std::string> query_params;

public:
    HttpRequest();
    ~HttpRequest();

    // Parsing functions

    bool VALID_CRLN_CRLN(const std::string &buffer);
    void storethebuffer(const std::string &buffer);
    void start_line();
    void headers();

    void parseRequestUri(const std::string &Uri);
    void parsebody(const std::string& buffer, size_t bytesReaded, size_t totalbytesReaded, std::ofstream& upload_file);
    void TransferEncoding(const std::string &buffer, size_t bytesReaded, std::ofstream& upload_file);
    void contentLength(const std::string &buffer, size_t bytesReaded, std::ofstream& upload_file, size_t EndofFile);

    void validstartline();
    bool validheader(const std::vector<std::string> &vheader);
    bool validbody(const std::string &buffer, size_t maxsize);

    static void split_header(const std::string &buffer, std::vector<std::string> &words);
    static void split_line(const std::string &buffer, std::vector<std::string> &words);

    // print
    static void print_vector(std::vector<std::string> &vec);
    static void print_map(const std::map<std::string, std::string> &m);

    // Getters
    std::string getMethod() const;
    std::string getDecodedPath() const;
    std::string getVersion() const;
    std::string GetHeaderContent(std::string HEADER);
    std::string GetHost();

    bool hasContentLength;
    bool hasTransferEncoding;

    void initBodyReadIndex(const std::string &buffer);
    void checkBodyCompletionOnEOF();
    size_t _Read_index_body;
    size_t content_length;
    size_t body_received;

    // For chunked decoding
    std::string chunk_size_line;
    size_t current_chunk_size;
    bool reading_chunk_size;
    bool reading_chunk_data;
    bool chunk_done;
};

std::string decodePercentEncoding(const std::string &path);
void parseParams(const std::string &query, std::map<std::string, std::string> Uri);
bool isBadUriTraversal(const std::string &uri);
bool isBadUri(const std::string &uri);
char hexToChar(char high, char low);
bool isHexDigit(char c);
std::string to_lowercase(const std::string& input);

