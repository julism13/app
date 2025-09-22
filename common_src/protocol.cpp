#include <iostream>
#include <arpa/inet.h>
#include <vector>
#include <algorithm>
#include "constants.h"
#include "protocol.h"
#include "socket.h"

uint16_t Protocol::to_big_endian_16(uint16_t value) {
    return htons(value);
}

uint32_t Protocol::to_big_endian_32(uint32_t value) {
    return htonl(value);
}

uint16_t Protocol::from_big_endian_16(uint16_t value) {
    return ntohs(value);
}

uint32_t Protocol::from_big_endian_32(uint32_t value) {
    return ntohl(value);
}

void Protocol::send_username(Socket& socket, const std::string& username) {
    uint8_t code = SEND_USERNAME;
    uint16_t length = to_big_endian_16(username.length());

    socket.sendall(&code, sizeof(code));
    socket.sendall(&length, sizeof(length));
    socket.sendall(username.c_str(), username.length());
}

std::string Protocol::get_username(Socket& socket) {
    uint16_t length;

    int bytes_read = socket.recvall(&length, sizeof(length));
    if (bytes_read == 0) {
        throw std::runtime_error("Connection closed during username length read");
    }
    
    length = from_big_endian_16(length);
    
    if (length > 100) {
        throw std::runtime_error("Username too long");
    }
    
    std::string username(length, '\0');
    bytes_read = socket.recvall(&username[0], length);
    if (bytes_read == 0) {
        throw std::runtime_error("Connection closed during username read");
    }

    return username;
}

void Protocol::send_get_current_car(Socket& socket) {
    uint8_t code = GET_CURRENT_CAR;
    socket.sendall(&code, sizeof(code));
}

void Protocol::send_get_market_info(Socket& socket) {
    uint8_t code = GET_MARKET_INFO;
    socket.sendall(&code, sizeof(code));
}

uint32_t Protocol::get_initial_money(Socket& socket) {
    uint8_t code;
    uint32_t money;
    
    int bytes_read = socket.recvall(&code, sizeof(code));
    if (bytes_read == 0) {
        throw std::runtime_error("Connection closed during money code read");
    }
    
    bytes_read = socket.recvall(&money, sizeof(money));
    if (bytes_read == 0) {
        throw std::runtime_error("Connection closed during money read");
    }
    
    return from_big_endian_32(money);
}

void Protocol::send_error_message(Socket& socket, const std::string& message) {
    uint8_t code = SEND_ERROR_MESSAGE;
    uint16_t length = to_big_endian_16(message.length());
    
    socket.sendall(&code, sizeof(code));
    socket.sendall(&length, sizeof(length));
    socket.sendall(message.c_str(), message.length());
}

std::string Protocol::get_error_message(Socket& socket) {
    uint16_t length;
    
    socket.recvall(&length, sizeof(length));
    length = from_big_endian_16(length);
    std::string message(length, '\0');
    socket.recvall(&message[0], length);
    
    return message;
}

uint8_t Protocol::get_command(Socket& socket) {
    uint8_t code;
    int bytes_read = socket.recvall(&code, sizeof(code));
    if (bytes_read == 0) {
        throw std::runtime_error("Connection closed");
    }
    return code;
}

void Protocol::buy_car(Socket& socket, const std::string& car_name) {
    uint8_t code = BUY_CAR;
    uint16_t length = to_big_endian_16(car_name.length());

    socket.sendall(&code, sizeof(code));
    socket.sendall(&length, sizeof(length));
    socket.sendall(car_name.c_str(), car_name.length());
}

std::string Protocol::get_car_name(Socket& socket) {
    uint16_t length;
    
    int bytes_read = socket.recvall(&length, sizeof(length));
    if (bytes_read == 0) {
        throw std::runtime_error("Connection closed during car name length read");
    }
    
    length = from_big_endian_16(length);
    if (length > 100) {
        throw std::runtime_error("Invalid car name length");
    }
    
    std::string car_name(length, '\0');
    bytes_read = socket.recvall(&car_name[0], length);
    if (bytes_read == 0) {
        throw std::runtime_error("Connection closed during car name read");
    }
    
    return car_name;
}

void Protocol::send_initial_money(Socket& socket, uint32_t money) {
    uint8_t code = SEND_INITIAL_MONEY;
    uint32_t money_be = to_big_endian_32(money);
    
    socket.sendall(&code, sizeof(code));
    socket.sendall(&money_be, sizeof(money_be));
}

void Protocol::send_current_car(Socket& socket, const std::string& name, uint16_t year, uint32_t price) {
    uint8_t code = SEND_CURRENT_CAR;
    
    socket.sendall(&code, sizeof(code));
    send_car_info(socket, name, year, price);
}

void Protocol::send_market_info(Socket& socket, const std::vector<Car>& market_order) {
    uint8_t code = SEND_MARKET_INFO;
    uint16_t num_cars = to_big_endian_16(market_order.size());
    
    socket.sendall(&code, sizeof(code));
    socket.sendall(&num_cars, sizeof(num_cars));
    
    for (const Car& car : market_order) {
        send_car_info(socket, car.name, car.year, car.price);
    }
}

void Protocol::send_car_info(Socket& socket, const std::string& name, uint16_t year, uint32_t price) {
    uint16_t name_length = to_big_endian_16(name.length());
    uint16_t year_be = to_big_endian_16(year);
    uint32_t price_be = to_big_endian_32(price * 100);
    
    socket.sendall(&name_length, sizeof(name_length));
    socket.sendall(name.c_str(), name.length());
    socket.sendall(&year_be, sizeof(year_be));
    socket.sendall(&price_be, sizeof(price_be));
}

Car Protocol::get_car_info(Socket& socket) {
    Car car;
    uint16_t name_length;
    uint16_t year;
    uint32_t price_int;
    
    int bytes_read = socket.recvall(&name_length, sizeof(name_length));
    if (bytes_read == 0) {
        throw std::runtime_error("Connection closed during car name length read");
    }
    
    name_length = from_big_endian_16(name_length);
    
    if (name_length > 100) {
        throw std::runtime_error("Car name too long");
    }
    
    car.name.resize(name_length);
    bytes_read = socket.recvall(&car.name[0], name_length);
    if (bytes_read == 0) {
        throw std::runtime_error("Connection closed during car name read");
    }
    
    bytes_read = socket.recvall(&year, sizeof(year));
    if (bytes_read == 0) {
        throw std::runtime_error("Connection closed during car year read");
    }
    car.year = from_big_endian_16(year);
    
    bytes_read = socket.recvall(&price_int, sizeof(price_int));
    if (bytes_read == 0) {
        throw std::runtime_error("Connection closed during car price read");
    }
    price_int = from_big_endian_32(price_int);
    car.price = price_int / 100;
    
    return car;
}

std::vector<Car> Protocol::get_market_info(Socket& socket) {
    std::vector<Car> market; 
    uint16_t num_cars;
    
    int bytes_read = socket.recvall(&num_cars, sizeof(num_cars));
    if (bytes_read == 0) {
        throw std::runtime_error("Connection closed during market size read");
    }
    
    num_cars = from_big_endian_16(num_cars);
    if (num_cars > 100) {
        throw std::runtime_error("Too many cars in market");
    }
    
    for (uint16_t i = 0; i < num_cars; i++) {
        Car car = get_car_info(socket);
        market.push_back(car);
    }
    
    return market;
}

void Protocol::send_car_bought(Socket& socket, const std::string& name, uint16_t year, uint32_t price, uint32_t remaining_money) {
    uint8_t code = SEND_CAR_BOUGHT;
    uint32_t remaining_money_be = to_big_endian_32(remaining_money);
    
    socket.sendall(&code, sizeof(code));
    send_car_info(socket, name, year, price);
    socket.sendall(&remaining_money_be, sizeof(remaining_money_be));
}

Car Protocol::get_car_bought(Socket& socket, uint32_t& remaining_money) {
    Car car = get_car_info(socket);
    uint32_t money;
    
    socket.recvall(&money, sizeof(money));
    remaining_money = from_big_endian_32(money);
    
    return car;
}
