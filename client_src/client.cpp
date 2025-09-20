#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>

#include "client.h"
#include "../common_src/protocol.h"
#include "../common_src/constants.h"
#include "../common_src/socket.h"

void Client::leer_script(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;

    while (std::getline(file, line)) {
        // Ignorar líneas vacías
        if (line.empty()) continue;
        
        Command cmd;
        std::istringstream iss(line);
        iss >> cmd.action;

        std::string resto;
        if (std::getline(iss, resto)) {
            if (!resto.empty() && resto[0] == ' ') {
                resto = resto.substr(1);
            }
            cmd.parameter = resto;
        }

        commands.push_back(cmd);
    }
}

void Client::run(const std::string& hostname, const std::string& port) {
    try {
        Socket socket(hostname.c_str(), port.c_str());
        execute_commands(socket);
    } catch (const std::exception& e) {
        // Conexión cerrada o error - terminar normalmente
        return;
    }
}

void Client::execute_commands(Socket& socket) {
    for (const Command& cmd : commands) {
        try {
            if (cmd.action == "username") {
                protocol.send_username(socket, cmd.parameter);
                
                uint32_t money = protocol.get_initial_money(socket);
                std::cout << "Initial balance: " << money << std::endl;
                
            } else if (cmd.action == "get_current_car") {
                protocol.send_get_current_car(socket);
                handle_server_response(socket);
                
            } else if (cmd.action == "get_market") {
                protocol.send_get_market_info(socket);
                handle_server_response(socket);
                
            } else if (cmd.action == "buy_car") {
                protocol.buy_car(socket, cmd.parameter);
                handle_server_response(socket);
            }
        } catch (const std::exception& e) {
            // Error en comunicación - terminar
            return;
        }
    }
    // Todos los comandos procesados - cliente termina
}

void Client::handle_server_response(Socket& socket) {
    uint8_t response_code = protocol.get_command(socket);
    
    switch (response_code) {
        case GET_CURRENT_CAR:  // El servidor envía el auto actual
            handle_current_car_response(socket);
            break;
        case SEND_MARKET_INFO:
            handle_market_response(socket);
            break;
        case SEND_CAR_BOUGHT:
            handle_car_bought_response(socket);
            break;
        case SEND_ERROR_MESSAGE:
            handle_error_response(socket);
            break;
        default:
            // Código desconocido - terminar
            break;
    }
}

void Client::handle_current_car_response(Socket& socket) {
    Car car = protocol.get_car_info(socket);
    std::cout << "Current car: " << car.name << ", year: " << car.year 
              << ", price: " << car.price << ".00" << std::endl;
}

void Client::handle_market_response(Socket& socket) {
    std::map<std::string, Car> market = protocol.get_market_info(socket);
    
    for (const auto& pair : market) {
        const Car& car = pair.second;
        std::cout << car.name << ", year: " << car.year << ", price: " 
                  << car.price << ".00" << std::endl;
    }
}

void Client::handle_car_bought_response(Socket& socket) {
    uint32_t remaining_money;
    Car car = protocol.get_car_bought(socket, remaining_money);
    
    std::cout << "Car bought: " << car.name << ", year: " << car.year 
              << ", price: " << car.price << ".00" << std::endl;
    std::cout << "Remaining balance: " << remaining_money << std::endl;
}

void Client::handle_error_response(Socket& socket) {
    std::string error = protocol.get_error_message(socket);
    std::cout << "Error: " << error << std::endl;
}
