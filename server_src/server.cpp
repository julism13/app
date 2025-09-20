#include <fstream>
#include <iostream>

#include "server.h"
#include "../common_src/protocol.h"
#include "../common_src/constants.h"
#include "../common_src/socket.h"

void Server::leer_config(const std::string& filename) {
    std::ifstream file(filename);
    std::string command;

    while (file >> command) {
        if (command == "money") {
            file >> initial_money;
        } else if (command == "car") {
            Car car;
            file >> car.name >> car.year >> car.price;
            market[car.name] = car;
        }
    }
    player_money = initial_money;
}

void Server::run(const std::string& port) {
    Socket acceptor(port.c_str());

    std::cout << "Server started" << std::endl;

    Socket client = acceptor.accept();
    handle_client(client);
}

void Server::handle_client(Socket& client) {
    try {
        // Recibir nombre de usuario
        std::string username = protocol.get_username(client);
        std::cout << "Hello, " << username << std::endl;
        
        // Enviar dinero inicial
        protocol.send_initial_money(client, initial_money);
        std::cout << "Initial balance: " << initial_money << std::endl;
        
        // Manejar comandos del cliente
        while (true) {
            uint8_t command;
            try {
                command = protocol.get_command(client);
            } catch (const std::exception& e) {
                // Cliente desconectado
                break;
            }
            
            switch (command) {
                case SEND_CURRENT_CAR:  // El cliente pide el auto actual
                    handle_get_current_car(client);
                    break;
                case GET_MARKET_INFO:
                    handle_get_market(client);
                    break;
                case BUY_CAR:
                    handle_buy_car(client);
                    break;
                default:
                    // Comando desconocido, probablemente fin de comunicación
                    return;
            }
        }
    } catch (const std::exception& e) {
        // Error de conexión, terminar silenciosamente
        return;
    }
}

void Server::handle_get_current_car(Socket& client) {
    try {
        if (!player_has_car) {
            protocol.send_error_message(client, "No car bought");
            std::cout << "Error: No car bought" << std::endl;
        } else {
            protocol.send_current_car(client, player_car.name, player_car.year, player_car.price);
            std::cout << "Car " << player_car.name << " " << (player_car.price * 100) << " " 
                      << player_car.year << " sent" << std::endl;
        }
    } catch (const std::exception& e) {
        throw; // Re-lanzar para terminar la conexión
    }
}

void Server::handle_get_market(Socket& client) {
    try {
        protocol.send_market_info(client, market);
        std::cout << market.size() << " cars sent" << std::endl;
    } catch (const std::exception& e) {
        throw; // Re-lanzar para terminar la conexión
    }
}

void Server::handle_buy_car(Socket& client) {
    try {
        std::string car_name = protocol.get_car_name(client);
        
        auto it = market.find(car_name);
        if (it == market.end()) {
            protocol.send_error_message(client, "Car not found");
            std::cout << "Error: Car not found" << std::endl;
            return;
        }
        
        const Car& car = it->second;
        if (player_money < car.price) {
            protocol.send_error_message(client, "Insufficient funds");
            std::cout << "Error: Insufficient funds" << std::endl;
            return;
        }
        
        // Comprar el auto
        player_money -= car.price;
        player_car = car;
        player_has_car = true;
        
        protocol.send_car_bought(client, car.name, car.year, car.price, player_money);
        std::cout << "New cars name: " << car.name << " --- remaining balance: " 
                  << player_money << std::endl;
                  
    } catch (const std::exception& e) {
        throw; // Re-lanzar para terminar la conexión
    }
}
