#pragma once
#include "route.hpp"
#include "Server.hpp"




class confugParser
{
    private:
        std::vector<Server*> fileData;
        typedef std::string (confugParser::*mapToListedElement)(std::string , Server*, std::ifstream&);
        typedef void (confugParser::*mapToLinearElement)(std::vector <std::string>&, Server*);
        std::map<int, std::map<std::string, mapToListedElement> > ListedMap;
        std::map<int, std::map<std::string, mapToLinearElement> > LinearMap;

        /////parsing functions (methods) for linear methods (confug file) /////
        void parseHost(std::vector<std::string> &line, Server *newServer);
        void parsePort(std::vector<std::string> &line, Server *newServer);
        void parseCBSL(std::vector<std::string> &line, Server *newServer);
        void parseServerName(std::vector<std::string> &line, Server *newServer);
        // void parsePaths(std::vector<std::string> &line, Server *newServer);
        // void parseMethods(std::vector<std::string> &line, Server *newServer);
        
        /////parsing functions (methods) for listed methods (confug file) /////
        std::string parseDERPAGES(std::string line, Server *newServer, std::ifstream &file);
        std::string parseRoute(std::string line, Server *newServer, std::ifstream &file);
        // std::string parseCGI(std::string line, Server *newServer, std::ifstream &file);

        void ServerParser(Server *newServer);

        //////// init map of pointers to member functions (parsing functions )
        void InitMapsFunctions(void);

        ///// parsing helpers ///////////////////////

        std::vector<std::string> splitBySpaces(const std::string str);

        //////// error while parsing ///////////////
        void Error(const std::vector<std::string> &words);

       

        
    public:
        confugParser();
        void Parser(const std::string &PathToConfig);
        
        std::vector<Server*> GetAllData();
        ~confugParser();
         /////// adding a client to server ....
        void newClient(int clientSocket, int ServerSocket);
        void removeClient(int socket);

};

