#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

#include <algorithm> // for std::find

class route
{
    private:
        std::map<std::string, std::string> paths;
        std::map<std::string, std::string> cgi;
        std::vector<std::string> index_file;
        std::vector<std::string> methods;
        bool auto_index;

    public:
        //////// seters ////////
        void SetPaths(std::string key, std::string value);
        void SetMethods(std::string value);
        void Setcgi(std::string key, std::string value);
        void SetAutoIndex(void);
        ///////// geters //////////
        std::map<std::string, std::string> GetPats(void);
        std::vector<std::string> GetMethods(void);
        std::map<std::string, std::string> GetCGI();
        bool GetAutoIndex(void);
        std::vector<std::string> getIndexFiles();
        void SetIndexFile(std::vector<std::string> &words);
        route(/* args */);
        ~route();
};