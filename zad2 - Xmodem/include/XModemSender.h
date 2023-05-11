#ifndef UNTITLED_XMODEMSENDER_H
#define UNTITLED_XMODEMSENDER_H

#include <fstream>
#include <iostream>

using namespace std;

class XModemSender {
private:
    istream &inputStream;
    ostream &outputStream;
public:
    XModemSender(istream &inputStream, ostream &outputStream);
    void sendFile(const string &fileName);
    void sendPacket(char data[], int length, int blockNumber);
    void sendEOT();
};
#endif //UNTITLED_XMODEMSENDER_H
