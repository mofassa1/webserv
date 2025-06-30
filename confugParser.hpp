#pragma once
#include "route.hpp"
#include "Server.hpp"

#include <csignal>
#include <cstdlib>
#include <iostream>

class confugParser
{
    private:
        std::vector<Server*> fileData;
        typedef std::string (confugParser::*mapToListedElement)(std::string , Server*, std::ifstream&);
        typedef void (confugParser::*mapToLinearElement)(std::vector <std::string>&, Server*);
        std::map<int, std::map<std::string, mapToListedElement> > ListedMap;
        std::map<int, std::map<std::string, mapToLinearElement> > LinearMap;

        void parseHost(std::vector<std::string> &line, Server *newServer);
        void parsePort(std::vector<std::string> &line, Server *newServer);
        void parseCBSL(std::vector<std::string> &line, Server *newServer);
        void parseServerName(std::vector<std::string> &line, Server *newServer);

        std::string parseDERPAGES(std::string line, Server *newServer, std::ifstream &file);
        std::string parseRoute(std::string line1, Server *newServer, std::ifstream &file);

        void ServerParser(Server *newServer);

        void InitMapsFunctions(void);

        std::vector<std::string> splitBySpaces(const std::string str);

        void Error(const std::vector<std::string> &words);

       

        
    public:
        confugParser();
        void Parser(const std::string &PathToConfig);
        
        std::vector<Server*> GetAllData();
        ~confugParser();
        void newClient(int clientSocket, int ServerSocket);
        void removeClient(int socket);

};

