#include <iostream>

#include "client.h"

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Uso: ./client <hostname> <puerto> <archivo-comandos>" << std::endl;
        return 1;
    }

    try {
        Client client;
        client.leer_script(argv[3]);
        client.run(argv[1], argv[2]);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}