#ifndef UNTITLED_SERIALPORT_H
#define UNTITLED_SERIALPORT_H

#include <windows.h>

class SerialPort {

private:
    HANDLE handleCom;
    BOOL isReadyPort;
    DCB controlDCB;
public:
    void initialize(char *chosenPort);
};


#endif //UNTITLED_SERIALPORT_H