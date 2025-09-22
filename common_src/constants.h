#include <cstdint>

#ifndef CONSTANTS_H
#define CONSTANTS_H

const uint8_t SEND_USERNAME = 0x01;
const uint8_t SEND_INITIAL_MONEY = 0x02;
const uint8_t SEND_CURRENT_CAR = 0x03; 
const uint8_t GET_CURRENT_CAR = 0x04;
const uint8_t GET_MARKET_INFO = 0x05;
const uint8_t SEND_MARKET_INFO = 0x06;
const uint8_t BUY_CAR = 0x07;
const uint8_t SEND_CAR_BOUGHT = 0x08;
const uint8_t SEND_ERROR_MESSAGE = 0x09;

#endif
