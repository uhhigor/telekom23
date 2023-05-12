#include <string>
#include <chrono>
#include <stdexcept>
#include <cstring>
#include <thread>
#include "SerialPort.h"
#include "variables.h"
#include "CalculateCheckSum.h"

SerialPort::SerialPort(char *chosenPort) {
    handleCom = CreateFile(chosenPort, GENERIC_READ | GENERIC_WRITE, 0,
                           nullptr, OPEN_EXISTING, 0, nullptr);
    //SetupComm(handleCom, 1, 128);
    GetCommState(handleCom, &controlDCB);
    controlDCB.BaudRate = BaudRate;
    controlDCB.ByteSize = 8;
    controlDCB.Parity = NOPARITY;
    controlDCB.StopBits = ONESTOPBIT;
    controlDCB.fParity = TRUE;
    controlDCB.fDtrControl = DTR_CONTROL_DISABLE;
    controlDCB.fRtsControl = RTS_CONTROL_DISABLE;
    controlDCB.fOutxCtsFlow = FALSE;
    controlDCB.fOutxDsrFlow = FALSE;
    controlDCB.fDsrSensitivity = FALSE;
    controlDCB.fAbortOnError = FALSE;
    controlDCB.fOutX = FALSE;
    controlDCB.fInX = FALSE;
    controlDCB.fErrorChar = FALSE;
    controlDCB.fNull = FALSE;
    SetCommState(handleCom, &controlDCB);
}
DWORD bytesRead, bytesWritten;

void SerialPort::sendFile(const string &fileName) {
    // Open file for reading
    if (handleCom == INVALID_HANDLE_VALUE)
    {
        throw std::runtime_error("Unable to open file: " + fileName);
    }

    // Wait for NAK from receive
    BYTE type;
    bool nakReceived = false;
    auto startTime = chrono::system_clock::now();
    while (chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - startTime).count() < TRANSMISSION_TIMEOUT) {
        ReadFile(handleCom, &type, 1, &bytesRead, nullptr);
        if (bytesRead > 0) {   //Checks if there is anything to read
            if (type == NAK || type == C) {
                nakReceived = true;
                break;
            }
        }
        this_thread::sleep_for(chrono::milliseconds(RESPONSE_TIMEOUT));
    }
    if (!nakReceived) {
        throw runtime_error("Timeout waiting for NAK response from receiver");
    }

    //Check transmission type CRC or not
    int additionalBlockLength;
    if(C == type)
        additionalBlockLength = 5;
    else
        additionalBlockLength = 4;

    // Send file contents
    int blockNumber = 1;
    char packetData[PACKET_SIZE];
    while (ReadFile(handleCom, packetData, PACKET_SIZE, &bytesRead, nullptr) && bytesRead > 0) {
        bytesRead = 0;
        // Send packet
        sendPacket(packetData,bytesRead, blockNumber, additionalBlockLength);

        // Increment block number
        blockNumber++;

        // Wait for ACK or NAK response
        bool ackReceived = false;
        bool nakReceivedAgain = false;
        startTime = chrono::system_clock::now();
        while (chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - startTime).count() < RESPONSE_TIMEOUT) {
            BYTE pom;
            bytesRead = 0;
            if (ReadFile(handleCom, &pom, 1, &bytesRead, nullptr) && bytesRead > 0){
                if (pom == ACK) {
                    ackReceived = true;
                    break;
                } else if (pom == NAK) {
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
        BYTE pom;
        bytesRead = 0;
        if (ReadFile(handleCom, &pom, 1, &bytesRead, nullptr) && bytesRead > 0) {
            if (pom == ACK) {              //if ACK returned, close transmission
                eotAckReceived = true;
                break;
            }
        }
    }
    if (!eotAckReceived) {
        throw runtime_error("Timeout waiting for ACK response from receiver after EOT");
    }
    CloseHandle(handleCom);
}

void SerialPort::sendPacket(char *data, DWORD length, int blockNumber, int additionalBlockLength) {
    char packet[PACKET_SIZE + additionalBlockLength];

    // Calculate checksum
    CalculateCheckSum calculateCheckSum;

    if (additionalBlockLength == 5) {
        uint16_t crc = calculateCheckSum.calculateCRC16(data, length);
        packet[PACKET_SIZE + 3] = static_cast<char>((crc >> 8) & 0xFF);
        packet[PACKET_SIZE + 4] = static_cast<char>(crc & 0xFF);
    } else {
        char checksum = (char) calculateCheckSum.calculateCheckSum(data, length);
        packet[PACKET_SIZE + 3] = checksum;
    }

    // Construct packet header and copy data
    packet[0] = SOH; // Start of Header
    packet[1] = (char) blockNumber;
    packet[2] = (char) ~blockNumber;
    memcpy(packet + 3, data, length);

    // Send packet
    WriteFile(handleCom, packet, PACKET_SIZE + additionalBlockLength, &bytesWritten, nullptr);
    if (bytesWritten != PACKET_SIZE + additionalBlockLength) {
        throw runtime_error("Failed to write data to serial port");
    }
}

void SerialPort::sendEOT() {
    WriteFile(handleCom, &EOT, 1, &bytesWritten, nullptr);
}

void SerialPort::receiveFile(const string &fileName) {
    // Open file for writing
    if (handleCom == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Failed to open serial port");
    }
    bool isCrcSupported;
    int additionalBlockLength;

    if(isCrcSupported){
        WriteFile(handleCom, &ACK, 1, nullptr, nullptr);
        additionalBlockLength = 5;
    } else {
        // Send NAK to sender
        WriteFile(handleCom, &NAK, 1, nullptr, nullptr);
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
        auto startTime = chrono::system_clock::now();
        while (chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - startTime).count() < 1000) {
            // Check if there is enough data available to read a packet
            DWORD bytesAvailable = 0;
            PeekNamedPipe(handleCom, nullptr, 0, nullptr, &bytesAvailable, nullptr);
            ReadFile(handleCom, packet, PACKET_SIZE + additionalBlockLength, &bytesRead, nullptr);
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
                            WriteFile(handleCom, packet + 3, PACKET_SIZE, &bytesWritten, nullptr);
                            blockNumber++;
                            WriteFile(handleCom, &ACK, 1, nullptr, nullptr);
                        } else {
                            // Checksum is invalid
                            WriteFile(handleCom, &NAK, 1, nullptr, nullptr);
                        }
                    } else {
                        checksum = (char) calculateCheckSum.calculateCheckSum(data, PACKET_SIZE);
                        if (checksum == packet[PACKET_SIZE + additionalBlockLength - 1]) {
                            // Checksum is valid
                            WriteFile(handleCom, packet + 3, PACKET_SIZE, &bytesWritten, nullptr);
                            blockNumber++;
                            WriteFile(handleCom, &ACK, 1, nullptr, nullptr);
                        } else {
                            // Checksum is invalid
                            WriteFile(handleCom, &NAK, 1, nullptr, nullptr);
                        }
                    }
                } else if (bytesRead == 1 && packet[0] == EOT) {
                    // End of file received
                    eofReceived = true;
                    break;
                } else {
                    // Invalid packet received
                    WriteFile(handleCom, &NAK, 1, nullptr, nullptr);
                }
        }
        if (bytesRead == 0) {
            // Timeout waiting for packet
            throw runtime_error("Timeout waiting for packet from sender");
        }
    }

    // Send ACK to sender
    WriteFile(handleCom, &ACK, 1, nullptr, nullptr);

    // Close file output stream
    CloseHandle(handleCom);
}


