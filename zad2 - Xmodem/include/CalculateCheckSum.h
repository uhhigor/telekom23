#ifndef UNTITLED_CALCULATECHECKSUM_H
#define UNTITLED_CALCULATECHECKSUM_H
#include <iostream>

using namespace std;

class CalculateCheckSum {
public:
    static uint16_t calculateCRC16(unsigned char *data, int length);
    static int calculateCheckSum(unsigned char *data, int length);
};


#endif //UNTITLED_CALCULATECHECKSUM_H