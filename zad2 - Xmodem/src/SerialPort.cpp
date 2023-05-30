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
bool nakReceived = false;
bool isPacket;

void SerialPort::sendFile(const string &fileName) {
    ifstream file;
    file.open(fileName, ios::in | ios::binary );
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

    isPacket = false;
    bool ackReceived = false;
    bool nakReceivedAgain = false;

    // Send file contents
    int blockNumber = 1;
    char packetData[PACKET_SIZE];
    while (true) {
        fill_n(packetData, PACKET_SIZE, ' '); //Clear values
        file.read(packetData, PACKET_SIZE);
        int bytesRead = file.gcount();
        if (bytesRead == 0) {
            // End of file reached, break the loop
            break;
        }

        // Send packet
        sendPacket(packetData,PACKET_SIZE, blockNumber, additionalBlockLength);

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

        // Increment block number
        blockNumber++;
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
    WriteFile(handleCom, packet, length, &bitsLengthInChar, nullptr);
    if (bitsLengthInChar != length) {
        throw runtime_error("Failed to write data to serial port");
    }

    // Calculate checksum
    CalculateCheckSum calculateCheckSum;

    if (additionalBlockLength == 2) {
        uint16_t crc = calculateCheckSum.calculateCRC16s(data, length);
        checksum[0] = static_cast<char>((crc >> 8) & 0xFF);
        checksum[1] = static_cast<char>(crc & 0xFF);
        WriteFile(handleCom, checksum, 2, &bitsLengthInChar, nullptr);
    } else {
        char checkSum = (char) calculateCheckSum.calculateCheckSums(data, length);
        checksum[0] = checkSum;
        WriteFile(handleCom, checksum, 1, &bitsLengthInChar, nullptr);
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
            std::fill(received, received + sizeof received, SUB);
            ReadFile(handleCom, &received, messageLen, &bitsLengthInChar, nullptr);
        }
        first = false;

        if(received[0] == SOH || received[0] == C) {
            cout << "\nReceiving packet "<<blockNumber<<"\n";

            cout << "HEADER 0: " << (int) received[0] << "\n";
            cout << "HEADER 1: " <<(int) received[1] << " == " << blockNumber <<"\n";
            cout << "HEADER 2: " << (int)received[2]  << " == " << 255-blockNumber <<"\n";

            if(received[1] == blockNumber && received[2] == 255-blockNumber) {
                cout << "Valid packet\n";
                //valid packet received
                uint8_t data[PACKET_SIZE];
                uint8_t checkSumReceived[additionalBlockLength];

                memcpy(&data, &received[3], PACKET_SIZE);
                memcpy(&checkSumReceived, &received[sizeof received - additionalBlockLength], additionalBlockLength);

                /*cout << "DATA: " << data <<"\n";
                for(unsigned char i : data)
                    cout<< (int) i<<" ";
                cout<<"\n";*/

                if (additionalBlockLength == 2) {
                    cout << "CRC\n";

                    uint8_t crcChecksum[2];
                    uint16_t crc = CalculateCheckSum::calculateCRC16(data, PACKET_SIZE);
                    crcChecksum[0] = uint8_t((crc >> 8) & 0xFF);
                    crcChecksum[1] = uint8_t(crc & 0xFF);

                    cout << "CHECKSUM 0: " << (int) checkSumReceived[0] << " == " << (int)crcChecksum[0];
                    cout << "\nCHECKSUM 1: " << (int) checkSumReceived[1] << " == " << (int)crcChecksum[1];
                    if (crcChecksum[0] == checkSumReceived[0]
                    && crcChecksum[1] == checkSumReceived[1]) {
                        blockNumber++;
                        cout<<"\nChecksum is valid";
                        WriteFile(handleCom, &ACK, 1, &bitsLengthInChar, nullptr);

                        //remove padding
                        int lastIndex = PACKET_SIZE - 1;
                        while (lastIndex >= 0 && data[lastIndex] == SUB)
                            lastIndex--;

                        int charArraySize = lastIndex + 1;

                        char charArray[charArraySize];
                        memcpy(charArray, data, charArraySize);
                        charArray[charArraySize] = '\0';

                        file.write(charArray, charArraySize);
                    } else {
                        cout<<"\nChecksum is invalid";
                        WriteFile(handleCom, &NAK, 1, &bitsLengthInChar, nullptr);
                    }
                } else {
                    uint8_t checkSum;
                    cout << "CHECKSUM\n";
                    checkSum = CalculateCheckSum::calculateCheckSum(data, sizeof data);

                    cout << "CHECKSUM 0: " << (int) checkSumReceived[0] << " == " << (int) checkSum;
                    if (checkSum == checkSumReceived[0]) {
                        blockNumber++;
                        cout << "\nChecksum is valid";
                        WriteFile(handleCom, &ACK, 1, &bitsLengthInChar, nullptr);

                        //remove padding
                        int lastIndex = PACKET_SIZE - 1;
                        while (lastIndex >= 0 && data[lastIndex] == SUB)
                            lastIndex--;

                        int charArraySize = lastIndex + 1;

                        char charArray[charArraySize];
                        memcpy(charArray, data, charArraySize);
                        charArray[charArraySize] = '\0';

                        file.write(charArray, charArraySize);
                    } else {
                        cout << "\nChecksum is invalid";
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
            cout<<"==========\nEND OF TRANSMISSION\n==========";
        }
        else throw runtime_error("Error");
    }
    file.close();
    // Send ACK to sender
    WriteFile(handleCom, &ACK, 1, &bitsLengthInChar, nullptr);

    // Close file output stream
    CloseHandle(handleCom);
}



