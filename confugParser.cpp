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
    std::cout << "distructor called !!!!!!!" << std::endl;
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
    /////// linear methodes (confug file)

    this->LinearMap[2]["host:"] = &confugParser::parseHost;
    this->LinearMap[2]["port:"] = &confugParser::parsePort;
    this->LinearMap[2]["client_body_size_limit:"] = &confugParser::parseCBSL;
    this->LinearMap[2]["server_name:"] = &confugParser::parseServerName;

    // this->LinearMap[4]["path:"] = &confugParser::parsePaths;
    // this->LinearMap[4]["directory:"] = &confugParser::parsePaths;
    // this->LinearMap[4]["index_file:"] = &confugParser::parsePaths;
    // this->LinearMap[4]["upload_directory:"] = &confugParser::parsePaths;

    /////// listed methodes (confug file)

    this->ListedMap[2]["default_error_pages:"] = &confugParser::parseDERPAGES;
    this->ListedMap[2]["route:"] = &confugParser::parseRoute;
    // this->ListedMap[4]["cgi:"] = &confugParser::parseCGI;
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
        /////// I need here to check is all data exist (finished parsing) , if not i have to throw an exiption 
        this->fileData.push_back(newServer);
    }
    newServer = new Server();
}

void confugParser::Error(const std::vector<std::string> &words)
{
    std::cerr << "Error at line : " << std::endl << " '";

    for (size_t i = 0; i < words.size(); i++)
        std::cerr << words[i];
    std::cerr << " '" << std::endl;
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

std::string confugParser::parseRoute(std::string line, Server *newServer, std::ifstream &file)
{
    route Newoute;
    int spacesCount;
    std::string inerscoop = "route:";

    std::string PathsKeys[4] = {
        "path:" ,"directory:", "index_file:" ,"upload_directory:"
    };

    while (std::getline(file, line))
    {
        std::vector<std::string> words;

        spacesCount = CountSpaces(line);
        if (spacesCount < 4)
        {
            //// i have to check in the route containing all the data before pushing it ///////
            newServer->SetRoute(Newoute);
            return line;
        }

        words = splitBySpaces(line);
        if (words.empty() || words[0].empty())
            continue;
        
        if (spacesCount == 4 && words[0] == "methods:")
        {
            if (words.size() == 1)
                std::runtime_error(words[0] + " : must have a value !!!!!!!!!!");
            for (size_t i = 0; i < words.size(); i++)
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
        else if (spacesCount == 4)
        {
            if (words.size() != 2)
                std::runtime_error("error : the key word must be followed by one arguiment");
            int i = 0;
            for (; i < 4; i++)
            {
                if (PathsKeys[i] == words[0])
                    break ;
            }
            if (PathsKeys[i] != words[0])
                std::runtime_error("Invalid key word !!!");
            
            Newoute.SetPaths(words[0], words[1]);
        }
        else if (spacesCount == 6 && inerscoop == "cgi" && words.size() == 2 && words[0][0] == '.')
        {
            std::cerr << " 66666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666"  << std::endl;
            std::cerr << "cgi is setting with key : ' " << words[0] << " '" << "and the value :  " << words[1] << std::endl;
            Newoute.Setcgi(words[0], words[1]);
        }
        else
            std::runtime_error("unown error in parsing !!!!!!!!!!");
    }

    //// i have to check in the route containing all the data before pushing it ///////
    newServer->SetRoute(Newoute);
    if (file.eof())
        return "";
    return line;
}


void confugParser::parseServerName(std::vector<std::string> &line, Server *newServer){
    // need work here , i will just store the value right now

    if (newServer->GetServerName() != "")
    {
        std::cerr << "SeconfugParser::rver name doubled !!!!!!!!!!" << std::endl;
        Error(line);
    }


    newServer->SetServerName(line[1]);
}

void confugParser::parseCBSL(std::vector<std::string> &line, Server *newServer)
{
    if (line.size() != 2)
    {
        std::cerr << "Invalid arguiments count" << std::endl;
        Error(line);
    }

    if (newServer->Getclient_body_size_limit() != 0)
    {
        std::cerr << "client_body_size_limit doubled !!!!" << std::endl;
        Error(line);
    }
    size_t value = stringToint(line[1]);

    if (value <= 0)
        Error(line);
    
    newServer->Setclient_body_size_limit(value);
}

void confugParser::parseHost(std::vector<std::string> &line, Server *newServer)
{
    // if (!newServer)
    //     throw std::runtime_error("the new server pointer is nullllll ");
    // int dots = 0;
    // int DigitIndex = 0;

    // if (line.size() != 2)
    //     Error(line);
    
    // for (size_t i = 0; i < line[1].size(); i++)
    // {
    //     if (dots > 3)
    //         Error(line);
    //     if (line[1][i] == '.')
    //     {
    //         DigitIndex = 0;
    //         dots++;
    //         if (i == 0)
    //             Error(line);
    //         if (!isdigit(line[1][i - 1]) || !isdigit(line[1][i + 1]))
    //             Error(line);
    //     }
    //     else if (!isdigit(line[1][i]))
    //             Error(line);
    //     else
    //     {
    //         if (DigitIndex > 2)
    //             Error(line);
    //         if (DigitIndex == 0 && line[1][i] > 2)
    //         {

    //             Error(line);
    //         }
    //         if ((DigitIndex == 1 || DigitIndex == 2) && line[1][i] > 5)
    //             Error(line);
    //         DigitIndex++;
    //     }
    // }
    
    // if (dots != 3)
    //     Error(line);

    newServer->SetHost(line[1]);
}

void confugParser::parsePort(std::vector<std::string> &line, Server *newServer)
{
    if (line.size() != 2)
        Error(line);
    size_t value = stringToint(line[1]);
    if (value > 65535)
        throw std::runtime_error("Invalid port: out of range (0-65535)");
    newServer->SetPorts(static_cast<unsigned short>(value));
}

void confugParser::Parser(const std::string &PathToConfig) {

    std::ifstream file(PathToConfig.c_str());
    std::string line;
    Server *newServer = 0;

    if (!file)
        throw std::runtime_error("Can't open the file !!!!!!!");

    while (std::getline(file, line))
    {
        start:
        // lineNumber++;
        // std::cerr << "line number is : " << lineNumber << " . the line is : " << line << std::endl;
        int spacesCount;
        std::vector<std::string> words;

        spacesCount = CountSpaces(line);
        if (spacesCount % 2)
            throw std::runtime_error("invalid spaces count");

        words = splitBySpaces(line);
        
        if (words.empty() || words[0].empty())
            continue;

        if (words.size() == 1 && words[0] == "server:")
        {
            std::cout << "#######################################################" << std::endl;
            std::cout << "the spaces count is : " << spacesCount << std::endl;
            if (spacesCount != 0)
                throw std::runtime_error("server should not start with spaces");
            if (newServer)
            {
                /////// I need here to check is all data exist (finished parsing) , if not i have to throw an exiption 
                this->fileData.push_back(newServer);
            }
            newServer = new Server();
            std::cout << "#######################################################" << std::endl;

        }
        else if (words.size() == 1)
        {
            if (ListedMap.count(spacesCount) && ListedMap[spacesCount].count(words[0]))
            {
                mapToListedElement func = ListedMap[spacesCount][words[0]];
                if (!func)
                    Error(words);
                line = (this->*func)(line, newServer, file);
                if (line == "")
                {
                    if (file.eof())
                        break ;
                }
                goto start;
            }
            else
                Error(words);
        }
        else
        {
            if (LinearMap.count(spacesCount) && LinearMap[spacesCount].count(words[0]))
            {
                mapToLinearElement func = LinearMap[spacesCount][words[0]];
                if (!func)
                    Error(words);
                (this->*func)(words, newServer);
            }
            else
                Error(words);
        }
    }

    if (newServer)
        ServerParser(newServer); 

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
