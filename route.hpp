#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>


class route
{
    private:
        std::map<std::string, std::string> paths;
        std::map<std::string, std::string> cgi;
        std::vector<std::string> methods;
    
    public:
        //////// seters ////////
        void SetPaths(std::string key, std::string value);
        void SetMethods(std::string value);
        void Setcgi(std::string key, std::string value);
        ///////// geters //////////
        std::map<std::string, std::string> GetPats(void);
        std::vector<std::string> GetMethods(void);
        std::map<std::string, std::string> GetCGI();

        route(/* args */);
        ~route();
};