#include <iostream>
#include <string>
#include <chrono>
#include <stdexcept>
#include <cstring>
#include "XModemReceiver.h"
#include "CalculateCheckSum.h"
#include "variables.h"

using namespace std;

XModemReceiver::XModemReceiver(istream &inputStream, ostream &outputStream) : inputStream(inputStream), outputStream(outputStream) {}

void XModemReceiver::receiveFile(const string& fileName) {
    // Open file for writing
    ofstream fileOutputStream(fileName, ios::binary);
    if (!fileOutputStream.is_open()) {
        throw runtime_error("Could not open file for writing");
    }
    bool isCrcSupported;
    int additionalBlockLength;

    if(isCrcSupported){
        outputStream.write(&C, 1);
        outputStream.flush();
        additionalBlockLength = 5;
    } else {
        // Send NAK to sender
        outputStream.write(&NAK, 1);
        outputStream.flush();
        additionalBlockLength = 4;
    }

    // Receive file contents
    int blockNumber = 1;
    bool eofReceived = false;
    char data[PACKET_SIZE];
    while (!eofReceived) {
        // Receive packet
        char packet[PACKET_SIZE + additionalBlockLength];
        memset(packet, 0, PACKET_SIZE + additionalBlockLength);
        int bytesRead = 0;
        auto startTime = chrono::system_clock::now();
        while (chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - startTime).count() < 1000) {
            //Checks if there is no end-of-file character in the input stream, the packet available
            if (inputStream.peek() != istream::traits_type::eof() && inputStream.rdbuf()->in_avail() >= PACKET_SIZE + additionalBlockLength) {
                bytesRead = inputStream.read(packet, PACKET_SIZE + additionalBlockLength).gcount();
                if (bytesRead == PACKET_SIZE + additionalBlockLength && packet[0] == SOH && packet[1] == blockNumber && packet[2] == ~blockNumber) {
                    // Valid packet received
                    memcpy(data, packet + 3, PACKET_SIZE);
                    CalculateCheckSum calculateCheckSum;
                    char checksum;
                    char crcChecksum[2];
                    if (additionalBlockLength == 5) {
                        uint16_t crc = calculateCheckSum.calculateCRC16(data, PACKET_SIZE);
                        crcChecksum[0] = static_cast<char>((crc >> 8) & 0xFF);
                        crcChecksum[1] = static_cast<char>(crc & 0xFF);
                        if (crcChecksum[0] == packet[PACKET_SIZE + additionalBlockLength - 2]
                            && crcChecksum[1] == packet[PACKET_SIZE + additionalBlockLength - 1]) {
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
                    } else {
                        checksum = (char) calculateCheckSum.calculateCheckSum(data, PACKET_SIZE);
                        if (checksum == packet[PACKET_SIZE + additionalBlockLength - 1]) {
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