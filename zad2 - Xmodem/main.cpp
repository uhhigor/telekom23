#include <iostream>
#include "SerialPort.h"

int main() {
    cout <<"Select the listening port: ";
    int portNumber;
    cin >> portNumber;
    string chosenPort ="COM" + to_string(portNumber);
    cout<<"Chosen port: " + chosenPort;

    cout <<"\nEnter the name of the file: ";
    string fileName;
    cin>>fileName;
    cout<<"Chosen file: " + fileName;

    cout<<"\nSelect the operating mode: ";
    int mode;
    cout<<"\n1. Sender, algebraic checksum";
    cout<<"\n2. Sender, CRC16 checksum";
    cout<<"\n3. Receiver, algebraic checksum";
    cout<<"\n4. Receiver, CRC16 checksum";
    cout<<"\nChoice: ";
    cin>>mode;
    cout<<"Chosen mode: " + to_string(mode);

    bool CRC16;
    SerialPort serialPort(chosenPort);
    switch (mode) {
        case 1:
            CRC16 = false;
            serialPort.sendFile(fileName, CRC16);
            break;
        case 2:
            CRC16 = true;
            serialPort.sendFile(fileName, CRC16);
            break;
        case 3:
            CRC16 = false;
            serialPort.receiveFile(fileName, CRC16);
            break;
        case 4:
            CRC16 = true;
            serialPort.receiveFile(fileName, CRC16);
            break;
        default:
            break;
    }
    return 0;
}