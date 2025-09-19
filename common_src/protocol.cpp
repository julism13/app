#include <iostream>
#include <arpa/inet.h>
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
    uint8_t code;
    uint16_t length;

    socket.recvall(&code, sizeof(code));
    socket.recvall(&length, sizeof(length));
    length = from_big_endian_16(length);
    std::string username(length, '\0');
    socket.recvall(&username[0], length);

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
    
    socket.recvall(&code, sizeof(code));
    socket.recvall(&money, sizeof(money));
    
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
    socket.recvall(&code, sizeof(code));
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
    
    socket.recvall(&length, sizeof(length));
    length = from_big_endian_16(length);
    std::string car_name(length, '\0');
    socket.recvall(&car_name[0], length);
    
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
    uint16_t name_length = to_big_endian_16(name.length());
    uint16_t year_be = to_big_endian_16(year);
    uint32_t price_be = to_big_endian_32(price);
    
    socket.sendall(&code, sizeof(code));
    socket.sendall(&name_length, sizeof(name_length));
    socket.sendall(name.c_str(), name.length());
    socket.sendall(&year_be, sizeof(year_be));
    socket.sendall(&price_be, sizeof(price_be));
}

void Protocol::send_market_info(Socket& socket, const std::map<std::string, Car>& market) {
    uint8_t code = SEND_MARKET_INFO;
    uint16_t num_cars = to_big_endian_16(market.size());
    
    socket.sendall(&code, sizeof(code));
    socket.sendall(&num_cars, sizeof(num_cars));
    
    for (const auto& pair : market) {
        const Car& car = pair.second;
        send_car_info(socket, car.name, car.year, car.price);
    }
}

void Protocol::send_car_info(Socket& socket, const std::string& name, uint16_t year, uint32_t price) {
    uint16_t name_length = to_big_endian_16(name.length());
    uint16_t year_be = to_big_endian_16(year);
    uint32_t price_be = to_big_endian_32(price);
    
    socket.sendall(&name_length, sizeof(name_length));
    socket.sendall(name.c_str(), name.length());
    socket.sendall(&year_be, sizeof(year_be));
    socket.sendall(&price_be, sizeof(price_be));
}

void Protocol::send_car_bought(Socket& socket, const std::string& name, uint16_t year, uint32_t price, uint32_t remaining_money) {
    uint8_t code = SEND_CAR_BOUGHT;
    uint32_t remaining_money_be = to_big_endian_32(remaining_money);
    
    socket.sendall(&code, sizeof(code));
    send_car_info(socket, name, year, price);
    socket.sendall(&remaining_money_be, sizeof(remaining_money_be));
}