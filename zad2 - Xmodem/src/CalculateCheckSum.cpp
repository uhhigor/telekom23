#include "CalculateCheckSum.h"

uint16_t CalculateCheckSum::calculateCRC16(unsigned char *data, int length) {
    uint16_t crc = 0;
    for (int i = 0; i < length; i++) {
        crc ^= ((uint16_t) data[i] << 8);
        for (int j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

int CalculateCheckSum::calculateCheckSum(unsigned char *data, int length) {
    int algebraicCheckSum = 0;
    unsigned char *ptr = data;

    while(length) {
        algebraicCheckSum += (*ptr++);
        length--;
    }
    algebraicCheckSum %= 256;
    return algebraicCheckSum;
}