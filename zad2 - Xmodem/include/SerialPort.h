#ifndef UNTITLED_SERIALPORT_H
#define UNTITLED_SERIALPORT_H

#include <windows.h>
#include <fstream>
#include <iostream>

using namespace std;

class SerialPort {

private:
    HANDLE handleCom;
    DCB controlDCB;
public:
    SerialPort(char *chosenPort);
    void sendFile(const string &fileName);
    void receiveFile(const string& fileName);
    void sendPacket(char data[], DWORD length, int blockNumber, int additionalBlockLength);
    void sendEOT();
};


#endif //UNTITLED_SERIALPORT_H