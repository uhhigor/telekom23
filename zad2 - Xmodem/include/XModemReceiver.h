#ifndef UNTITLED_XMODEMRECEIVER_H
#define UNTITLED_XMODEMRECEIVER_H

#include <fstream>
#include <iostream>

using namespace std;

class XModemReceiver{
private:
    istream &inputStream;
    ostream &outputStream;
public:
    XModemReceiver(istream& inputStream, ostream& outputStream);
    void receiveFile(const string& fileName);
};
#endif //UNTITLED_XMODEMRECEIVER_H
