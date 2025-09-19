#include <fstream>
#include <iostream>
#include <sstream>
#include <arpa/inet.h>

#include "client.h"
#include "../common_src/protocol.h"
#include "../common_src/constants.h"
#include "../common_src/socket.h"

void Client::leer_script(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;

    while (std::getline(file, line)) {
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
    Socket socket(hostname.c_str(), port.c_str());
    execute_commands(socket);
}

void Client::execute_commands(Socket& socket) {
    for (const Command& cmd : commands) {
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
    }
}

void Client::handle_server_response(Socket& socket) {
    uint8_t response_code = protocol.get_command(socket);
    
    switch (response_code) {
        case SEND_CURRENT_CAR:
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
            std::cout << "Unknown response code: " << (int)response_code << std::endl;
            break;
    }
}

void Client::handle_current_car_response(Socket& socket) {
    // Recibir info del auto
    uint16_t name_length;
    socket.recvall(&name_length, sizeof(name_length));
    name_length = ntohs(name_length);
    
    std::string name(name_length, '\0');
    socket.recvall(&name[0], name_length);
    
    uint16_t year;
    socket.recvall(&year, sizeof(year));
    year = ntohs(year);
    
    uint32_t price;
    socket.recvall(&price, sizeof(price));
    price = ntohl(price);
    
    std::cout << "Current car: " << name << ", year: " << year 
              << ", price: " << (price / 100.0) << std::endl;
}

void Client::handle_market_response(Socket& socket) {
    uint16_t num_cars;
    socket.recvall(&num_cars, sizeof(num_cars));
    num_cars = ntohs(num_cars);
    
    for (int i = 0; i < num_cars; i++) {
        // Recibir info de cada auto
        uint16_t name_length;
        socket.recvall(&name_length, sizeof(name_length));
        name_length = ntohs(name_length);
        
        std::string name(name_length, '\0');
        socket.recvall(&name[0], name_length);
        
        uint16_t year;
        socket.recvall(&year, sizeof(year));
        year = ntohs(year);
        
        uint32_t price;
        socket.recvall(&price, sizeof(price));
        price = ntohl(price);
        
        std::cout << name << ", year: " << year << ", price: " 
                  << (price / 100.0) << std::endl;
    }
}

void Client::handle_car_bought_response(Socket& socket) {
    // Recibir info del auto comprado
    uint16_t name_length;
    socket.recvall(&name_length, sizeof(name_length));
    name_length = ntohs(name_length);
    
    std::string name(name_length, '\0');
    socket.recvall(&name[0], name_length);
    
    uint16_t year;
    socket.recvall(&year, sizeof(year));
    year = ntohs(year);
    
    uint32_t price;
    socket.recvall(&price, sizeof(price));
    price = ntohl(price);
    
    uint32_t remaining_money;
    socket.recvall(&remaining_money, sizeof(remaining_money));
    remaining_money = ntohl(remaining_money);
    
    std::cout << "Car bought: " << name << ", year: " << year 
              << ", price: " << (price / 100.0) << std::endl;
    std::cout << "Remaining balance: " << remaining_money << std::endl;
}

void Client::handle_error_response(Socket& socket) {
    std::string error = protocol.get_error_message(socket);
    std::cout << "Error: " << error << std::endl;
}