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
    cout<<"\n1. Sender";
    cout<<"\n2. Receiver (algebraic)";
    cout<<"\n3. Receiver (CR16)";
    cout<<"\nChoice: ";
    cin>>mode;
    cout<<"Chosen mode: " + to_string(mode);

    SerialPort serialPort(chosenPort);
    switch (mode) {
        case 1:
            serialPort.sendFile(R"(C:\Users\Maciej\Repositories\telekom23\zad2 - Xmodem\)" + fileName);
            break;
        case 2:
            serialPort.receiveFile(R"(C:\Users\Maciej\Repositories\telekom23\zad2 - Xmodem\)" + fileName, false);
            break;
        case 3:
            serialPort.receiveFile(R"(C:\Users\Maciej\Repositories\telekom23\zad2 - Xmodem\)" + fileName, true);
            break;
        default:
            break;
    }
    return 0;
}