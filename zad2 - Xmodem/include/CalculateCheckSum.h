#ifndef UNTITLED_CALCULATECHECKSUM_H
#define UNTITLED_CALCULATECHECKSUM_H
#include <iostream>

using namespace std;

class CalculateCheckSum {
public:
    uint16_t calculateCRC16(char *data, int length);
    int calculateCheckSum(char *data, int length);
};


#endif //UNTITLED_CALCULATECHECKSUM_H