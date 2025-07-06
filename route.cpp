#include "route.hpp"

void route::SetMethods(std::string value)
{
    if (std::find(this->methods.begin(), this->methods.end(), value) != this->methods.end())
    {
        throw std::runtime_error(value + " is duplicated !!!");
    }
    this->methods.push_back(value);
}


void route::SetPaths(std::string key, std::string value)
{

   if (this->paths.find(key) != this->paths.end())
   {
        throw std::runtime_error(key + " is duplicated !!!");
   }
    this->paths[key] = value;
}

void route::Setcgi(std::string key, std::string value)
{
    if (this->cgi.find(key) != this->cgi.end())
    {
        throw std::runtime_error(key + " is duplicated !!!");
    }
    this->cgi[key] = value;
}

void route::SetAutoIndex(void){
    if (this->auto_index)
        throw std::runtime_error("duplicated setting of autoIndex !!");
    auto_index = true;
}
void route::SetIndexFile(std::vector<std::string> &words){
    for (size_t i = 0; i < words.size(); i++)
    {
        if (words[i][0] == '#')
            return ;
        index_file.push_back(words[i]);
    }
    
}


std::vector<std::string>& route::getIndexFiles(){
    return this->index_file;
}

std::map<std::string, std::string>& route::GetPats(void)
{
    return this->paths;
}
std::vector<std::string>& route::GetMethods(void)
{
    return this->methods;
}

std::map<std::string, std::string>& route::GetCGI()
{
    return this->cgi;
}

bool& route::GetAutoIndex(void){
    return this->auto_index;
}

route::route()
{
    this->auto_index = false;
}

route::~route()
{
    
}

route& route::operator=(const route& other)
{
    if (this != &other)
    {
        this->paths = other.paths;
        this->cgi = other.cgi;
        this->index_file = other.index_file;
        this->methods = other.methods;
        this->auto_index = other.auto_index;
    }
    return *this;
}
