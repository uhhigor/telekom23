#ifndef UNTITLED_VARIABLES_H
#define UNTITLED_VARIABLES_H

// XModem control characters
const BYTE SOH = 0x01; // Start Of Header
const BYTE EOT = 0x04; // End Of Transmission
const BYTE ACK = 0x06; // Acknowledge
const BYTE NAK = 0x15; // Negative Acknowledge
const BYTE C = 0x43; // CRC16 checksum

// XModem packet size
const int PACKET_SIZE = 128;

// Response timeout in milliseconds
const int RESPONSE_TIMEOUT = 10000;
const int TRANSMISSION_TIMEOUT = 60000;

// Speed of transmission
const int BaudRate = 9600;


#endif //UNTITLED_VARIABLES_H