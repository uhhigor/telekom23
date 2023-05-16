#include <string>
#include <chrono>
#include <stdexcept>
#include <cstring>
#include <thread>
#include <vector>
#include "SerialPort.h"
#include "variables.h"
#include "CalculateCheckSum.h"

SerialPort::SerialPort(const string& chosenPort)
{
    handleCom = CreateFile((R"(\\.\)" + chosenPort).c_str(), GENERIC_READ | GENERIC_WRITE, 0,
                           nullptr, OPEN_EXISTING, 0, nullptr);
    if (handleCom == INVALID_HANDLE_VALUE)
    {
        DWORD error = GetLastError();
        LPVOID lpMsgBuf;
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                      nullptr, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, nullptr);
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
    file.open(fileName, ios::in | ios::binary);
    if (file.is_open()) {
        cout << "\nFile opened successfully";
        if (file.peek() == ifstream::traits_type::eof()) {
            throw runtime_error("File is empty");
        }
    } else {
        throw runtime_error("\nFailed to open file: " + fileName);
    }

    // Wait for NAK from receive
    char type;

    while(true) {
        ReadFile(handleCom, &type, 1, &bitsLengthInChar, nullptr);
        if (bitsLengthInChar > 0) {
            if (type == NAK || type == C) {
                cout << "\nConnection has been established";
                nakReceived = true;
                break;
            } else if (!nakReceived) {
                throw runtime_error("Timeout waiting for NAK response from receiver");
            }
        }
    }

    int additionalBlockLength;
    if (type == C)
        additionalBlockLength = 2;
    else
        additionalBlockLength = 1;

    isPacket = false;

    // Send file contents
    size_t previousBlockNumber;
    size_t blockNumber = 1;
    std::vector<uint8_t> data(PACKET_SIZE);
    while (true) {
        if(blockNumber != previousBlockNumber) {
            file.read(reinterpret_cast<char *>(data.data()), PACKET_SIZE);
            std::streamsize bytesRead = file.gcount();
            if (bytesRead == 0) {
                // End of file reached, break the loop
                break;
            }
        }

        sendPacket(data,PACKET_SIZE, blockNumber, additionalBlockLength);

        // Wait for ACK or NAK response
        char pom;
        bitsLengthInChar = 0;
        while (true) {
            ReadFile(handleCom, &pom, 1, &bitsLengthInChar, nullptr);
            if (bitsLengthInChar > 0) {
                if (pom == ACK) {
                    cout << "\nACK\n";
                    isPacket = true;
                    previousBlockNumber = blockNumber;
                    blockNumber++;
                    break;
                } else if (pom == NAK) {
                    cout << "\nNAK\n";
                    previousBlockNumber = blockNumber;
                    break;
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
    file.close();
    CloseHandle(handleCom);
}

void SerialPort::sendPacket(std::vector<uint8_t> &data, int length, size_t blockNumber, int additionalBlockLength) {
    cout << "\nSending " << blockNumber << " packet";
    uint8_t packet[PACKET_SIZE + 3 + additionalBlockLength];
    packet[0] = SOH;
    packet[1] = static_cast<uint8_t>(blockNumber);
    packet[2] = static_cast<uint8_t>(~blockNumber);

    memcpy(&packet[3], data.data(), PACKET_SIZE);

    uint8_t checksum[additionalBlockLength];

    if (additionalBlockLength == 2) {
        uint16_t crc = CalculateCheckSum::calculateCRC16(data.data(), length);
        checksum[0] = static_cast<char>((crc >> 8) & 0xFF);
        checksum[1] = static_cast<char>(crc & 0xFF);
    } else {
        uint8_t checkSum = CalculateCheckSum::calculateCheckSum(data.data(), length);
        checksum[0] = checkSum;
    }

    memcpy(&packet[3+PACKET_SIZE], checksum, sizeof checksum);

    WriteFile(handleCom, packet, sizeof packet, &bitsLengthInChar, nullptr);


    if (bitsLengthInChar != sizeof packet) {
        throw runtime_error("Failed to write data to serial port");
    }
}

void SerialPort::sendEOT() {
    WriteFile(handleCom, &EOT, 1, &bitsLengthInChar, nullptr);
}


void SerialPort::receiveFile(const string &fileName, bool isCRCSupported) {

    // Open file for writing
    ofstream file(fileName, ios::out | ios::binary);
    if (file.is_open()) {
        cout << "\nFile opened successfully";
    } else {
        throw runtime_error("\nFailed to open file: " + fileName);
    }

    int additionalBlockLength;
    char type;

    cout<<"\nWaiting for SOH";
    if(isCRCSupported){
        type = C;
        additionalBlockLength = 2;
    } else {
        type = NAK;
        additionalBlockLength = 1;
    }
    bool first = true;
    DWORD messageLen = PACKET_SIZE+3+additionalBlockLength;
    uint8_t received[messageLen];
    for (int i = 0; i < 6; i++) {
        WriteFile(handleCom, &type, 1, &bitsLengthInChar, nullptr);
        std::this_thread::sleep_for(std::chrono::seconds(10));
        ReadFile(handleCom, &received, messageLen, &bitsLengthInChar, nullptr);
        if(bitsLengthInChar > 0)
            if(received[0] == SOH || received[0] == C)
                break;
    }

    bool eofReceived = false;
    size_t blockNumber = 1;
    while (!eofReceived) {
        cout<<"\nWaiting\n";
        if(!first) {
            std::fill(received, received + sizeof received, 0);
            ReadFile(handleCom, &received, messageLen, &bitsLengthInChar, nullptr);
        }
        first = false;

        if(received[0] == SOH || received[0] == C) {
            cout << "\nReceiving packet "<<blockNumber<<"\n";

            if(received[0] == SOH)
                additionalBlockLength = 1;
            else if(received[0] == C)
                additionalBlockLength = 2;

            cout << "0: " << (int) received[0] << "\n";
            cout << "1: " <<(int) received[1] << " == " << blockNumber <<"\n";
            cout << "2: " << (int)received[2]  << " == " << 255-blockNumber <<"\n";

            if(received[1] == blockNumber && received[2] == 255-blockNumber) {
                cout << "Valid packet\n";
                //valid packet received
                uint8_t data[PACKET_SIZE];
                uint8_t checkSumReceived[additionalBlockLength];

                memcpy(&data, &received[3], PACKET_SIZE);
                memcpy(&checkSumReceived, &received[sizeof received - additionalBlockLength], additionalBlockLength);

                cout << "DATA: " << data <<"\n";

                if (additionalBlockLength == 2) {
                    cout << "CRC\n";
                    cout << "CHECKSUM 0: " << (int) checkSumReceived[0];
                    cout << "CHECKSUM 1: " << (int) checkSumReceived[1];
                    char crcChecksum[2];
                    uint16_t crc = CalculateCheckSum::calculateCRC16(data, PACKET_SIZE);
                    crcChecksum[0] = static_cast<char>((crc >> 8) & 0xFF);
                    crcChecksum[1] = static_cast<char>(crc & 0xFF);
                    if (crcChecksum[0] == checkSumReceived[0]
                    && crcChecksum[1] == checkSumReceived[1]) {
                        blockNumber++;
                        cout<<"\nChecksum is valid";
                        WriteFile(handleCom, &ACK, 1, &bitsLengthInChar, nullptr);
                    } else {
                        cout<<"\nChecksum is invalid";
                        WriteFile(handleCom, &NAK, 1, &bitsLengthInChar, nullptr);
                    }
                } else {

                    uint8_t checkSum;
                    cout << "CHECKSUM\n";
                    checkSum = CalculateCheckSum::calculateCheckSum(data, sizeof data);

                    cout << "CHECKSUM 0: " << (int)checkSumReceived[0] << " == " << (int)checkSum;
                    if (checkSum == checkSumReceived[0]) {
                        blockNumber++;
                        cout<<"\nChecksum is valid";
                        WriteFile(handleCom, &ACK, 1, &bitsLengthInChar, nullptr);

                        file.write(reinterpret_cast<const char *>(data), sizeof data);
                    } else {
                        cout<<"\nChecksum is invalid";
                        WriteFile(handleCom, &NAK, 1, &bitsLengthInChar, nullptr);
                    }
                }
            } else {
                // Invalid packet received
                cout<<"\nInvalid packet";
                WriteFile(handleCom, &NAK, 1, &bitsLengthInChar, nullptr);
            }
        } else if (received[0] == EOT) {
            eofReceived = true;
            cout<<"\nEND OF TRANSMISSION\n";
        }
        else throw runtime_error("Error");
    }
    file.close();
    // Send ACK to sender
    WriteFile(handleCom, &ACK, 1, &bitsLengthInChar, nullptr);

    // Close file output stream
    CloseHandle(handleCom);
}



