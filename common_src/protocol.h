#include <cstdint>
#include <string>
#include <map>

#include "socket.h"

#ifndef PROTOCOL_H
#define PROTOCOL_H

// Forward declaration
struct Car {
    std::string name;
    uint16_t year;
    uint32_t price;

    Car() : name(""), year(0), price(0) {}
};

class Protocol {
public:
    void send_username(Socket& socket, const std::string& username);
    void send_initial_money(Socket& socket, uint32_t money);
    void send_get_current_car(Socket& socket);
    void send_current_car(Socket& socket, const std::string& name, uint16_t year, uint32_t price);
    void send_get_market_info(Socket& socket);
    void send_market_info(Socket& socket, const std::map<std::string, Car>& market);
    void buy_car(Socket& socket, const std::string& car_name);
    void send_car_bought(Socket& socket, const std::string& name, uint16_t year, uint32_t price,
                         uint32_t remaining_money);
    void send_error_message(Socket& socket, const std::string& message);
    void send_car_info(Socket& socket, const std::string& name, uint16_t year, uint32_t price);
    
    std::string get_error_message(Socket& socket);
    std::string get_username(Socket& socket);
    std::string get_car_name(Socket& socket);
    uint32_t get_initial_money(Socket& socket);
    uint8_t get_command(Socket& socket);

private:
    uint16_t to_big_endian_16(uint16_t value);
    uint32_t to_big_endian_32(uint32_t value);
    uint16_t from_big_endian_16(uint16_t value);
    uint32_t from_big_endian_32(uint32_t value);
};

#endif