#ifndef UNTITLED_SERIALPORT_H
#define UNTITLED_SERIALPORT_H

#include <windows.h>
#include <fstream>
#include <iostream>

using namespace std;

class SerialPort {

private:
    HANDLE handleCom;
public:
    explicit SerialPort(const string &chosenPort);
    void sendFile(const string &fileName, bool isCRCSupported);
    void receiveFile(const string& fileName, bool isCRCSupported);
    void sendPacket(char data[], DWORD length, int blockNumber, int additionalBlockLength);
    void sendEOT();
};


#endif //UNTITLED_SERIALPORT_H