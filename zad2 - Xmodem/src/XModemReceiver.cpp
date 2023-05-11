#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <cstring>
#include "XModemReceiver.h"
#include "variables.h"

using namespace std;

XModemReceiver::XModemReceiver(istream &inputStream, ostream &outputStream) : inputStream(inputStream), outputStream(outputStream) {}

void XModemReceiver::receiveFile(const string& fileName) {
    // Open file for writing
    ofstream fileOutputStream(fileName, ios::binary);

    // Send NAK to sender
    outputStream.write(&NAK, 1);
    outputStream.flush();

    // Receive file contents
    int blockNumber = 1;
    bool eofReceived = false;
    while (!eofReceived) {
        // Receive packet
        char packet[PACKET_SIZE + 3];
        memset(packet, 0, PACKET_SIZE + 3);
        int bytesRead = 0;
        auto startTime = chrono::system_clock::now();
        while (chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - startTime).count() < 1000) {
            if (inputStream.peek() != istream::traits_type::eof() && inputStream.rdbuf()->in_avail() >= PACKET_SIZE + 3) {
                bytesRead = inputStream.read(packet, PACKET_SIZE + 3).gcount();
                if (bytesRead == PACKET_SIZE + 3 && packet[0] == SOH && packet[1] == blockNumber && packet[2] == ~blockNumber) {
                    // Valid packet received
                    char checksum = 0;
                    for (int i = 0; i < PACKET_SIZE; i++) {
                        checksum += packet[i + 3];
                    }
                    if (checksum == packet[PACKET_SIZE + 3 - 1]) {
                        // Checksum is valid
                        fileOutputStream.write(packet + 3, PACKET_SIZE);
                        blockNumber++;
                        outputStream.write(&ACK, 1);
                        outputStream.flush();
                    } else {
                        // Checksum is invalid
                        outputStream.write(&NAK, 1);
                        outputStream.flush();
                    }
                } else if (bytesRead == 1 && packet[0] == EOT) {
                    // End of file received
                    eofReceived = true;
                    break;
                } else {
                    // Invalid packet received
                    outputStream.write(&NAK, 1);
                    outputStream.flush();
                }
            }
        }
        if (bytesRead == 0) {
            // Timeout waiting for packet
            throw runtime_error("Timeout waiting for packet from sender");
        }
    }

    // Send ACK to sender
    outputStream.write(&ACK, 1);
    outputStream.flush();

    // Close file output stream
    fileOutputStream.close();
}


