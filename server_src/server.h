#include <map>
#include <string>
#include <vector>
#include <algorithm>

#include "../common_src/protocol.h"
#include "../common_src/socket.h"

#ifndef SERVER_H
#define SERVER_H

class Server {
private:
    std::map<std::string, Car> market;
    std::vector<Car> market_order;  // Para mantener orden de inserción
    Protocol protocol;
    Car player_car;
    uint32_t initial_money;
    uint32_t player_money;
    bool player_has_car;

public:
    Server() : initial_money(0), player_money(0), player_has_car(false) {}

    void leer_config(const std::string& filename);
    void run(const std::string& port);
    void handle_client(Socket& client);
    void handle_get_current_car(Socket& client);
    void handle_get_market(Socket& client);
    bool handle_buy_car(Socket& client);
};

#endif
