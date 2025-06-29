#include "confugParser.hpp"


confugParser::confugParser()
{
    this->InitMapsFunctions();
}

std::vector<Server*> confugParser::GetAllData()
{
    return fileData;
}

confugParser::~confugParser()
{
    for (size_t i = 0; i < this->fileData.size(); i++)
    {
        delete fileData[i];
    }
}


int CountSpaces(std::string line)
{
    int i = 0;
    while (line[i] == ' ')
        i++;
    if (i % 2)
        throw std::runtime_error("Invalid spaces conte");
    return i;
}

void confugParser::InitMapsFunctions(void)
{
    this->LinearMap[2]["host:"] = &confugParser::parseHost;
    this->LinearMap[2]["port:"] = &confugParser::parsePort;
    this->LinearMap[2]["client_body_size_limit:"] = &confugParser::parseCBSL;
    this->LinearMap[2]["server_name:"] = &confugParser::parseServerName;
    this->ListedMap[2]["default_error_pages:"] = &confugParser::parseDERPAGES;
    this->ListedMap[2]["route:"] = &confugParser::parseRoute;
}

std::vector<std::string> confugParser::splitBySpaces(const std::string str) {
    std::vector<std::string> tokens;
    std::istringstream iss(str);
    std::string word;

    while (iss >> word)
    {
        if (word[0] == '#')
            break ;
        tokens.push_back(word);
    }

    return tokens;
}


void confugParser::ServerParser(Server *newServer)
{
    if (newServer)
    {
        this->fileData.push_back(newServer);
    }
}


void confugParser::Error(const std::vector<std::string> &words)
{
    for (size_t i = 0; i < words.size(); i++)
        std::cout << words[i];
    std::cout << " '" << std::endl;
    throw std::runtime_error("Syntax error");
}

size_t     stringToint(std::string line)
{
    if (line.empty())
        throw std::runtime_error("Port string is empty");

    for (size_t i = 0; i < line.size(); i++) {
        if (!isdigit(line[i]))
            throw std::runtime_error("Invalid value");
    }

    std::istringstream iss(line);
    size_t value;
    iss >> value;

    if (iss.fail() || !iss.eof())
        throw std::runtime_error("Invalid value: conversion failed");


    return value;
}

std::string confugParser::parseDERPAGES(std::string line, Server *newServer, std::ifstream &file){
    int ExpectedSpaces = 4;

    std::vector<std::string> words;
    while (std::getline(file, line))
    {

        if (CountSpaces(line) > ExpectedSpaces)
            throw std::runtime_error("Error in the 'default_error_pages' scoop, spaces count not correct");
        if (CountSpaces(line) < ExpectedSpaces)
            return line;
        words = splitBySpaces(line);
        if (words.size() != 2)
        {
            throw std::runtime_error("Error in the 'default_error_pages' scoop");
        }

        unsigned short value;
        std::istringstream iss(words[0]);
        iss >> value;
        if (iss.fail())
            throw std::runtime_error("Invalid value: conversion failed");

        newServer->SetDefaultERRPages(value, words[1]);

    }
    if (file.eof())
        return "";
    return line;
}

std::string confugParser::parseRoute(std::string line1, Server *newServer, std::ifstream &file)
{
    (void)line1;
    route Newoute;
    std::string line = "";
    int spacesCount;
    std::string inerscoop = "route:";


    while (std::getline(file, line))
    {
        spacesCount = CountSpaces(line);
        if (spacesCount < 4)
        {
            if (Newoute.GetMethods().empty() || Newoute.GetPats()["path:"].empty() || \
                    Newoute.GetPats()["directory:"].empty() ){
                    throw std::runtime_error("invalid route !!");
                    }
            newServer->SetRoute(Newoute);
            return line;
        }
        if (spacesCount != 6)
            inerscoop = "route:";
        std::vector<std::string> words;

        words = splitBySpaces(line);
        if (words.empty() || words[0].empty())
            continue;
        if (spacesCount == 4 && words[0] == "auto_index:")
        {
            if (words.size() == 1)
                throw std::runtime_error(words[0] + " must followed by a value !!!");
            if (words.size() > 2 && words[2][0] != '#')
                throw std::runtime_error(words[0] + " must be followed by one value");
            if (words[1] == "ON" || words[1] == "on")
                Newoute.SetAutoIndex();
            else if (words[1] != "OFF" && words[1] != "off")
                throw std::runtime_error("invalid value for 'auto_index:' keyWord !!" );
        }
        else if (spacesCount == 4 && words[0] == "methods:")
        {
            if (words.size() == 1)
                throw std::runtime_error(words[0] + " : must have a value !!!!!!!!!!");
            for (size_t i = 1; i < words.size(); i++)
            {
                Newoute.SetMethods(words[i]);
            }
        }
        else if (spacesCount == 4 && words[0] == "cgi:")
        {
            if (words.size() != 1)
                std::runtime_error("cgi key word must be followed by list");
            inerscoop = "cgi";
        }
        else if (spacesCount == 4 && (words[0] ==  "path:" || 
                words[0] ==  "directory:"  
                || words[0] == "upload_directory:" || words[0] == "redirect:"))
        {
            if ((words.size() > 2 && words[2][0] != '#') || words.size() == 1)
                std::runtime_error("error : the key word must be followed by one arguiment");
            Newoute.SetPaths(words[0], words[1]);
        }
        else if (spacesCount == 4 && words[0] ==  "index_file:")
        {
            if (words.size() < 2)
                std::runtime_error("error : in the index_file ");
            Newoute.SetIndexFile(words);
        }
        else if (spacesCount == 6 && inerscoop == "cgi" && words.size() == 2 )
        {
            if (words[0][0] != '.')
                throw std::runtime_error("syntax error");
            Newoute.Setcgi(words[0], words[1]);
        }
        else
            std::runtime_error("unown error in configuration file !!!!!!!!!!");
    }
    if (Newoute.GetMethods().empty() || Newoute.GetPats()["path:"].empty() || \
        Newoute.GetPats()["directory:"].empty() )
            throw std::runtime_error("invalid route !!");
    newServer->SetRoute(Newoute);
    if (file.eof())
        return "";
    return line;
}


void confugParser::parseServerName(std::vector<std::string> &line, Server *newServer){
    if (line.size() != 2 && line[2][0] != '#')
        throw std::runtime_error("server name accept only one value !!");

    if (newServer->GetServerName() != "")
    {
        Error(line);
    }


    newServer->SetServerName(line[1]);
}

void confugParser::parseCBSL(std::vector<std::string> &line, Server *newServer)
{
    if (line.size() == 1)
        throw std::runtime_error("No arguiment ");
    if (line.size() > 2 && line[2][0] != '#')
        throw std::runtime_error("invalid arguiments count !");
    if (newServer->Getclient_body_size_limit() != 0)
    {
        if (newServer)
            delete newServer;
        Error(line);
    }
    size_t value = stringToint(line[1]);

    if (value <= 0)
    {
        if (newServer)
            delete newServer;
        Error(line);
    }
    
    newServer->Setclient_body_size_limit(value);
}

void confugParser::parseHost(std::vector<std::string> &line, Server *newServer)
{
    newServer->SetHost(line[1]);
}

void confugParser::parsePort(std::vector<std::string> &line, Server *newServer)
{
    if (line.size() != 2)
    {
        if (newServer)
            delete newServer;
        Error(line);
    }
    size_t value = stringToint(line[1]);
    if (value > 65535)
        throw std::runtime_error("Invalid port: out of range (0-65535)");
    newServer->SetPorts(static_cast<unsigned short>(value));
}

bool validServer(std::vector<Server *> servers, Server *newServer){

    for (size_t i = 0; i < servers.size(); i++)
    {
        Server *current = servers[i];

        if (current->GetHost() == newServer->GetHost() && \
            current->GetServerName() == newServer->GetServerName())
            {
                for (size_t j = 0; j < current->GetPorts().size(); j++)
                {
                    unsigned short currentRoute = current->GetPorts()[j];

                    for (size_t k = 0; k < newServer->GetPorts().size(); k++)
                    {
                        if (currentRoute == newServer->GetPorts()[k])
                            throw std::runtime_error("two or more servers with the same port , server name and host");
                    }
                    
                }
                
            }
    }
    

    return (newServer->GetHost() != "" && !newServer->GetPorts().empty()
        && newServer->Getclient_body_size_limit() != 0 && newServer->GetServerName() != "" && \
        !newServer->GetRoute().empty());
}

void confugParser::Parser(const std::string &PathToConfig) {

    std::ifstream file(PathToConfig.c_str());
    std::string line;
    Server *newServer = 0;

    if (!file)
        throw std::runtime_error("Can't open the file !!!!!!!");
    file.seekg(0, std::ios::end); 
    if (file.tellg() == 0)
        throw std::runtime_error("The configuration file is empty!");

    file.seekg(0, std::ios::beg);

    while (std::getline(file, line))
    {
        start:
        int spacesCount;
        std::vector<std::string> words;

        spacesCount = CountSpaces(line);
        if (spacesCount % 2)
        {
            if (newServer)
                delete newServer;
            throw std::runtime_error("invalid spaces count");
        }

        words = splitBySpaces(line);
        
        if (words.empty() || words[0].empty())
            continue;

        if (words.size() == 1 && words[0] == "server:")
        {
            if (spacesCount != 0)
            {
                if (newServer)
                    delete newServer;
                throw std::runtime_error("server should not start with spaces");
            }
            if (newServer)
            {
                if (!validServer(fileData, newServer))
                {
                    if (newServer)
                        delete newServer;
                    throw std::runtime_error("invalid server !!!");
                }
                this->fileData.push_back(newServer);
            }
            newServer = new Server();
        }
        else if (words.size() == 1)
        {
            if (ListedMap.count(spacesCount) && ListedMap[spacesCount].count(words[0]))
            {
                mapToListedElement func = ListedMap[spacesCount][words[0]];
                if (!func)
                {
                    if (newServer)
                        delete newServer;
                    Error(words);
                }
                try
                {
                    line = (this->*func)(line, newServer, file);
                }
                catch(const std::exception& e)
                {
                    if (newServer)
                        delete newServer;
                    throw std::runtime_error(e.what());
                }
                if (line == "")
                {
                    if (file.eof())
                        break ;
                }
                goto start;
            }
            else
            {
                if (newServer)
                    delete newServer;
                Error(words);
            }
        }
        else
        {
            if (LinearMap.count(spacesCount) && LinearMap[spacesCount].count(words[0]))
            {
                mapToLinearElement func = LinearMap[spacesCount][words[0]];
                if (!func)
                {
                    if (newServer)
                        delete newServer;
                    Error(words);
                }
                try
                {
                        (this->*func)(words, newServer);            
                }
                catch(const std::exception& e)
                {
                        if (newServer)
                            delete newServer;
                        throw std::runtime_error(e.what());
                }
            }
            else
            {
                if (newServer)
                    delete newServer;
                Error(words);
            }
        }
    }
    if (!validServer(fileData, newServer))
    {
        if (newServer)
            delete newServer;
        throw std::runtime_error("invalid server !!!");
    }
    if (newServer)
        this->fileData.push_back(newServer);

    file.close();
}


void confugParser::newClient(int clientSocket, int ServerSocket){
    size_t count = this->fileData.size();
    for (size_t i = 0; i < count; i++)
    {
        if (this->fileData[i]->isTheSeverSocket(ServerSocket))
            this->fileData[i]->setClientSocket(clientSocket);
    }
}

void confugParser::removeClient(int socket){
    size_t count = this->fileData.size();

    for (size_t i = 0; i < count; i++)
    {
        
        this->fileData[i]->removeClient(socket);
    }
    
}
