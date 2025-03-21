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


route::route()
{

}

route::~route()
{
    std::cout << "route distructor called !!!!!!!!!!!!" << std::endl;
}
