#include "confugParser.hpp"
#include "Multiplexer.hpp"

int main(int ac, char** av)
{
    // int epollFd;

    if (ac != 2)
    {
        // std::cerr << "no confuguration file" << std::endl;
        return -1;
    }
    confugParser File;
    try
    {
        File.Parser(av[1]);
        
        Multiplexer multiplexer;
        multiplexer.startMultiplexing(File);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    
    return 0;
}
