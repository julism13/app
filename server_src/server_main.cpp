#include <iostream>

#include "server.h"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Uso: ./server <puerto> <archivo-config>" << std::endl;
        return 1;
    }

    try {
        Server server;
        server.leer_config(argv[2]);
        server.run(argv[1]);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}