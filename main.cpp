#include "confugParser.hpp"
#include "Multiplexer.hpp"

confugParser* g_conf = NULL;
Multiplexer* g_mux = NULL;


void handle_signal(int signum)
{
    std::cerr << "\n[INFO] Caught signal: " << signum << ", cleaning up...\n";
    if (g_mux)
    {
        delete g_mux;
        g_mux = NULL;
    }
    if (g_conf)
    {
        delete g_conf;
        g_conf = NULL;
    }
    std::exit(EXIT_FAILURE);
}

int main(int ac, char** av)
{
    if (ac != 2)
        return -1;

    // Register signal handlers
    std::signal(SIGINT, handle_signal);
    std::signal(SIGTERM, handle_signal);

    g_conf = new confugParser();
    try
    {
        g_conf->Parser(av[1]);

        g_mux = new Multiplexer();
        g_mux->startMultiplexing(*g_conf);

        // Cleanup after run finishes successfully
        delete g_mux;
        delete g_conf;
    }
    catch(const std::exception& e)
    {
        std::cerr << "[ERROR] " << e.what() << '\n';
        delete g_mux;
        delete g_conf;
        return 1;
    }

    return 0;
}
