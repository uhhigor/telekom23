#include <iostream>
#include <chrono>
#include <thread>
#include <cstring>
#include "XModemSender.h"
#include "CalculateCheckSum.h"
#include "variables.h"

// Input and output streams
XModemSender::XModemSender(istream &inputStream, ostream &outputStream) : inputStream(inputStream), outputStream(outputStream) {}

void XModemSender::sendFile(const string &fileName) {
    // Open file for reading
    ifstream fileInputStream(fileName, ios::binary);
    if (!fileInputStream) {
        throw runtime_error("Unable to open file: " + fileName);
    }


    // Wait for NAK from receiver
    bool nakReceived = false;
    auto startTime = chrono::system_clock::now();
    while (chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - startTime).count() < 60000) {
        if (inputStream.rdbuf()->in_avail() > 0) {
            int b = inputStream.get();
            if (b == NAK) {
                nakReceived = true;
                break;
            }
        }
        this_thread::sleep_for(chrono::milliseconds(10));
    }
    if (!nakReceived) {
        throw runtime_error("Timeout waiting for NAK response from receiver");
    }
    // Send file contents
    int blockNumber = 1;
    char packetData[PACKET_SIZE];
    int bytesRead;
    while ((bytesRead = fileInputStream.readsome(packetData, PACKET_SIZE)) > 0) {
        // Send packet
        sendPacket(packetData, bytesRead, blockNumber);
        // Increment block number
        blockNumber++;
        // Wait for ACK or NAK response
        bool ackReceived = false;
        bool nakReceivedAgain = false;
        startTime = chrono::system_clock::now();
        while (chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - startTime).count() <
               RESPONSE_TIMEOUT) {
            if (inputStream.rdbuf()->in_avail() > 0) {
                int b = inputStream.get();
                if (b == ACK) {
                    ackReceived = true;
                    break;
                } else if (b == NAK) {
                    nakReceivedAgain = true;
                    break;
                }
            }
        }
        if (!ackReceived) {
            if (nakReceivedAgain) {
                // Resend packet
                blockNumber--;
            } else {
                throw runtime_error("Timeout waiting for ACK/NAK response from receiver");
            }
        }
    }
    // Send EOT
    sendEOT();
    // Wait for ACK response
    bool eotAckReceived = false;
    startTime = chrono::system_clock::now();
    while (chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - startTime).count() <
           RESPONSE_TIMEOUT) {
        if (inputStream.rdbuf()->in_avail() > 0) {
            int b = inputStream.get();
            if (b == ACK) {
                eotAckReceived = true;
                break;
            }
        }
    }
    if (!eotAckReceived) {
        throw runtime_error("Timeout waiting for ACK response from receiver after EOT");
    }
    fileInputStream.close();
}

void XModemSender::sendPacket(char data[], int length, int blockNumber) {

    char packet[PACKET_SIZE + 5];
    bool CRC16;

    // Calculate checksum
    char checksum = 0;
    CalculateCheckSum calculateCheckSum;

    if (CRC16) {
        checksum = (char) calculateCheckSum.calculateCRC16(data, length);
        packet[PACKET_SIZE + 3] = (char) (checksum >> 8);
        packet[PACKET_SIZE + 4] = (char) checksum;
    } else {
        checksum = (char) calculateCheckSum.calculateCheckSum(data, length);
        packet[PACKET_SIZE + 3] = checksum;
    }

    // Construct packet header and copy data
    packet[0] = SOH; // Start of Header
    packet[1] = (char) blockNumber;
    packet[2] = (char) ~blockNumber;
    memcpy(packet + 3, data, length);

    // Send packet
    outputStream.write(packet, PACKET_SIZE + 5);
    outputStream.flush();
}

    void XModemSender::sendEOT() {
        outputStream.write(&EOT, 1);
        outputStream.flush();
    }