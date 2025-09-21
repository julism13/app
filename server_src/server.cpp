#include <fstream>
#include <iostream>
#include <arpa/inet.h>

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
            player_money = initial_money;
        } else if (command == "car") {
            Car car;
            file >> car.name >> car.year >> car.price;
            market[car.name] = car;
            market_order.push_back(car);  // Mantener orden de inserción
        }
    }
}

void Server::run(const std::string& port) {
    Socket acceptor(port.c_str());
    std::cout << "Server started" << std::endl;

    Socket client = acceptor.accept();
    handle_client(client);
}

void Server::handle_client(Socket& client) {
    try {
        // Leer el código SEND_USERNAME primero
        uint8_t command_code;
        int bytes_read = client.recvsome(&command_code, sizeof(command_code));
        if (bytes_read == 0) {
            return;
        }
        
        if (command_code != SEND_USERNAME) {
            return;
        }
        
        // Ahora leer el username (sin código, ya lo leímos)
        uint16_t length;
        bytes_read = client.recvall(&length, sizeof(length));
        if (bytes_read == 0) return;
        
        length = ntohs(length);
        if (length > 100) return;
        
        std::string username(length, '\0');
        bytes_read = client.recvall(&username[0], length);
        if (bytes_read == 0) return;
        
        std::cout << "Hello, " << username << std::endl;
        
        // Enviar dinero inicial
        protocol.send_initial_money(client, initial_money);
        std::cout << "Initial balance: " << initial_money << std::endl;
        
        // Procesar comandos hasta que el cliente se desconecte
        while (true) {
            uint8_t command;
            
            // Usar recvsome para detectar desconexión inmediatamente
            int bytes_received = client.recvsome(&command, sizeof(command));
            if (bytes_received == 0) {
                // Cliente cerró la conexión - terminar normalmente
                break;
            }
            
            switch (command) {
                case GET_CURRENT_CAR:  // 0x04 (según tests esperados)
                    handle_get_current_car(client);
                    break;
                case GET_MARKET_INFO:  // 0x05  
                    handle_get_market(client);
                    break;
                case BUY_CAR:  // 0x07
                    if (!handle_buy_car(client)) {
                        // Error en la compra, posible desconexión
                        return;
                    }
                    break;
                default:
                    // Comando desconocido, terminar
                    return;
            }
        }
    } catch (const std::exception& e) {
        // Error de comunicación, terminar silenciosamente
    }
}

void Server::handle_get_current_car(Socket& client) {
    if (!player_has_car) {
        protocol.send_error_message(client, "No car bought");
        std::cout << "Error: No car bought" << std::endl;
    } else {
        protocol.send_current_car(client, player_car.name, player_car.year, player_car.price);
        std::cout << "Car " << player_car.name << " " << player_car.price 
                  << " " << player_car.year << " sent" << std::endl;
    }
}

void Server::handle_get_market(Socket& client) {
    // Usar el vector que preserva el orden de inserción
    std::map<std::string, Car> ordered_market;
    for (const Car& car : market_order) {
        ordered_market[car.name] = car;
    }
    
    protocol.send_market_info(client, ordered_market);
    std::cout << market.size() << " cars sent" << std::endl;
}

bool Server::handle_buy_car(Socket& client) {
    try {
        std::string car_name = protocol.get_car_name(client);
        
        auto it = market.find(car_name);
        if (it == market.end()) {
            protocol.send_error_message(client, "Car not found");
            std::cout << "Error: Car not found" << std::endl;
            return true; // Error manejado correctamente
        }
        
        const Car& car = it->second;
        if (player_money < car.price) {
            protocol.send_error_message(client, "Insufficient funds");
            std::cout << "Error: Insufficient funds" << std::endl;
            return true; // Error manejado correctamente
        }
        
        // Comprar auto
        player_money -= car.price;
        player_car = car;
        player_has_car = true;
        
        protocol.send_car_bought(client, car.name, car.year, car.price, player_money);
        std::cout << "New cars name: " << car.name << " --- remaining balance: " 
                  << player_money << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        // Error de comunicación (probablemente desconexión)
        return false;
    }
}
