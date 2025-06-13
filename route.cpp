#include "route.hpp"

///////// Seters .//////
void route::SetMethods(std::string value)
{
    this->methods.push_back(value);
}

void route::SetPaths(std::string key, std::string value)
{
    this->paths[key] = value;
}

void route::Setcgi(std::string key, std::string value)
{
    this->cgi[key] = value;
}

void route::SetAutoIndex(void){
    if (this->auto_index)
        throw std::runtime_error("duplicated setting of autoIndex !!");
    auto_index = true;
}

/// ////// Geters //////////

std::map<std::string, std::string> route::GetPats(void)
{
    return this->paths;
}
std::vector<std::string> route::GetMethods(void)
{
    return this->methods;
}

std::map<std::string, std::string> route::GetCGI()
{
    return this->cgi;
}

bool route::GetAutoIndex(void){
    return this->auto_index;
}

route::route()
{
    this->auto_index = false;
}

route::~route()
{
    // std::cout << "route distructor called !!!!!!!!!!!!" << std::endl;
}
