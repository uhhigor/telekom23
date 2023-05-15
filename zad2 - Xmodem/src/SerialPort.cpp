#include <string>
#include <chrono>
#include <stdexcept>
#include <cstring>
#include <thread>
#include "SerialPort.h"
#include "variables.h"
#include "CalculateCheckSum.h"

SerialPort::SerialPort(const string& chosenPort)
{
    handleCom = CreateFile((R"(\\.\)" + chosenPort).c_str(), GENERIC_READ | GENERIC_WRITE, 0,
                           NULL, OPEN_EXISTING, 0, NULL);
    if (handleCom == INVALID_HANDLE_VALUE)
    {
        DWORD error = GetLastError();
        LPVOID lpMsgBuf;
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                      NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL);
        string errorMessage((LPTSTR) lpMsgBuf);
        LocalFree(lpMsgBuf);
        throw runtime_error("Failed to open serial port " + chosenPort + ": " + errorMessage);
    }

    // Set up communication parameters
    DCB dcb = { 0 };
    dcb.DCBlength = sizeof(dcb);
    if (!GetCommState(handleCom, &dcb))
    {
        throw runtime_error("Failed to get communication state");
    }
    dcb.BaudRate = CBR_9600;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    dcb.fParity = TRUE;
    dcb.fDtrControl = DTR_CONTROL_DISABLE;
    dcb.fRtsControl = RTS_CONTROL_DISABLE;
    dcb.fOutxCtsFlow = FALSE;
    dcb.fOutxDsrFlow = FALSE;
    dcb.fDsrSensitivity = FALSE;
    dcb.fAbortOnError = FALSE;
    dcb.fOutX = FALSE;
    dcb.fInX = FALSE;
    dcb.fErrorChar = FALSE;
    dcb.fNull = FALSE;
    if (!SetCommState(handleCom, &dcb))
    {
        throw runtime_error("Failed to set communication state");
    }
    COMMTIMEOUTS  portTimings{0};
    portTimings.ReadIntervalTimeout = 10000;
    portTimings.ReadTotalTimeoutMultiplier = 10000;
    portTimings.ReadTotalTimeoutConstant = 10000;
    portTimings.WriteTotalTimeoutMultiplier = 100;
    portTimings.WriteTotalTimeoutConstant = 100;
    SetCommTimeouts(handleCom, &portTimings);
}

unsigned long bitsLengthInChar;
bool isPacket;
bool nakReceived = false;

void SerialPort::sendFile(const string &fileName, bool isCRCSupported) {
    ifstream file;
    file.open(fileName, ios::in | ios::out | ios::binary);
    if (file.is_open()) {
        cout << "\nFile opened successfully";
    } else {
        throw runtime_error("\nFailed to open file: " + fileName);
    }

    // Wait for NAK from receive
    char type;
    while(true) {
        ReadFile(handleCom, &type, 1, &bitsLengthInChar, nullptr);
        if (bitsLengthInChar > 0) {   //Checks if there is anything to read
            if (type == NAK || type == C) {
                cout << "\nConnection has been established";
                nakReceived = true;
                break;
            } else if (!nakReceived) {
                throw runtime_error("Timeout waiting for NAK response from receiver");
            }
        }
    }

    //Check transmission type CRC or not
    int additionalBlockLength;
    if (C == type)
        additionalBlockLength = 2;
    else
        additionalBlockLength = 1;

    // Send file contents
    int blockNumber = 1;
    char packetData[PACKET_SIZE];
    while (!file.eof()) {
        // Read from file
        for (int i = 0; i < PACKET_SIZE; i++)
            packetData[i] = (char) 26;
        int w = 0;
        while (w < PACKET_SIZE && !file.eof()) {
            packetData[w] = file.get();
            cout << "\nPacket Data: " << packetData[w];
            if (file.eof()) packetData[w] = (char) 26;
            w++;
        }
        isPacket = false;
        bool ackReceived = false;
        bool nakReceivedAgain = false;
        while (!isPacket) {
            // Send packet
            sendPacket(packetData,PACKET_SIZE, blockNumber, additionalBlockLength);

            // Increment block number
            blockNumber++;

            // Wait for ACK or NAK response

            char pom;
            bitsLengthInChar = 0;
            while (true) {
                if (ReadFile(handleCom, &pom, 1, &bitsLengthInChar, nullptr) && bitsLengthInChar > 0) {
                    if (pom == ACK) {
                        isPacket = true;
                        ackReceived = true;
                        break;
                    } else if (pom == NAK) {
                        nakReceivedAgain = true;
                        break;
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
            }
        }

    // Send EOT
    sendEOT();

    // Wait for ACK response
    bool eotAckReceived = false;
    char pom;
    bitsLengthInChar = 0;
    if (ReadFile(handleCom, &pom, 1, &bitsLengthInChar, nullptr) && bitsLengthInChar > 0) {
        if (pom == ACK) {

            eotAckReceived = true;
        }
    }
    if (!eotAckReceived) {
        throw runtime_error("Timeout waiting for ACK response from receiver after EOT");
    }
    cout << "\nTransmission complete";
    CloseHandle(handleCom);
}

void SerialPort::sendPacket(char *data, int length, int blockNumber, int additionalBlockLength) {
    cout<<"\nSending " << blockNumber << " packet";
    char header[3];
    char packet[PACKET_SIZE];
    char checksum[additionalBlockLength];

    // Construct packet header and copy data
    header[0] = SOH; // Start of Header
    header[1] = (char) blockNumber;
    header[2] = (char) ~blockNumber;
    WriteFile(handleCom, header, 3, &bitsLengthInChar, nullptr);
    memcpy(packet, data, length);

    // Send packet
    WriteFile(handleCom, packet, PACKET_SIZE, &bitsLengthInChar, nullptr);
    if (bitsLengthInChar != PACKET_SIZE) {
        throw runtime_error("Failed to write data to serial port");
    }

    // Calculate checksum
    CalculateCheckSum calculateCheckSum;

    if (additionalBlockLength == 2) {
        uint16_t crc = calculateCheckSum.calculateCRC16(data, length);
        checksum[0] = static_cast<char>((crc >> 8) & 0xFF);
        checksum[1] = static_cast<char>(crc & 0xFF);
        WriteFile(handleCom, checksum, 2, &bitsLengthInChar, nullptr);
    } else {
        char checkSum = (char) calculateCheckSum.calculateCheckSum(data, length);
        checksum[0] = checkSum;
        WriteFile(handleCom, checksum, 1, &bitsLengthInChar, nullptr);
    }
}


void SerialPort::sendEOT() {
    WriteFile(handleCom, &EOT, 1, &bitsLengthInChar, nullptr);
}

void SerialPort::receiveFile(const string &fileName, bool isCRCSupported) {
    // Open file for writing
    ofstream file(fileName, ios::in | ios::out | ios::binary);;
    if (file.is_open()) {
        cout<<"\nFile opened successfully";
    } else {
        throw runtime_error("Failed to open file: " + fileName);
    }

    int additionalBlockLength;

    if(isCRCSupported){
        WriteFile(handleCom, &ACK, 1, &bitsLengthInChar, nullptr);
        additionalBlockLength = 5;
    } else {
        // Send NAK to sender
        WriteFile(handleCom, &NAK, 1, &bitsLengthInChar, nullptr);
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
            ReadFile(handleCom, packet, PACKET_SIZE + additionalBlockLength, &bitsLengthInChar, nullptr);
                if (bitsLengthInChar == PACKET_SIZE + additionalBlockLength && packet[0] == SOH && packet[1] == blockNumber && packet[2] == ~blockNumber) {
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
                            WriteFile(handleCom, packet + 3, PACKET_SIZE, &bitsLengthInChar, nullptr);
                            blockNumber++;
                            WriteFile(handleCom, &ACK, 1, &bitsLengthInChar, nullptr);
                        } else {
                            // Checksum is invalid
                            WriteFile(handleCom, &NAK, 1, &bitsLengthInChar, nullptr);
                        }
                    } else {
                        checksum = (char) calculateCheckSum.calculateCheckSum(data, PACKET_SIZE);
                        if (checksum == packet[PACKET_SIZE + additionalBlockLength - 1]) {
                            // Checksum is valid
                            WriteFile(handleCom, packet + 3, PACKET_SIZE, &bitsLengthInChar, nullptr);
                            blockNumber++;
                            WriteFile(handleCom, &ACK, 1, &bitsLengthInChar, nullptr);
                        } else {
                            // Checksum is invalid
                            WriteFile(handleCom, &NAK, 1, &bitsLengthInChar, nullptr);
                        }
                    }
                } else if (bitsLengthInChar == 1 && packet[0] == EOT) {
                    // End of file received
                    eofReceived = true;
                    break;
                } else {
                    // Invalid packet received
                    WriteFile(handleCom, &NAK, 1, &bitsLengthInChar, nullptr);
                }
        }
        if (bitsLengthInChar == 0) {
            // Timeout waiting for packet
            throw runtime_error("Timeout waiting for packet from sender");
        }
    }

    // Send ACK to sender
    WriteFile(handleCom, &ACK, 1, &bitsLengthInChar, nullptr);

    // Close file output stream
    CloseHandle(handleCom);
}



