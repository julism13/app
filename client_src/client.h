#include <string>
#include <vector>

#include "../common_src/protocol.h"
#include "../common_src/socket.h"

#ifndef CLIENT_H
#define CLIENT_H

struct Command {
    std::string action;
    std::string parameter;
};

class Client {
private:
    std::vector<Command> commands;
    Protocol protocol;

public:
    void leer_script(const std::string& filename);
    void run(const std::string& hostname, const std::string& port);
    void execute_commands(Socket& socket);
    void handle_server_response(Socket& socket);
    void handle_current_car_response(Socket& socket);
    void handle_market_response(Socket& socket);
    void handle_car_bought_response(Socket& socket);
    void handle_error_response(Socket& socket);
};

#endif
