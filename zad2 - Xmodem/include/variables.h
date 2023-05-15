#ifndef UNTITLED_VARIABLES_H
#define UNTITLED_VARIABLES_H

// XModem control characters
const char SOH = 0x01; // Start Of Header
const char EOT = 0x04; // End Of Transmission
const char ACK = 0x06; // Acknowledge
const char NAK = 0x15; // Negative Acknowledge
const char C = 0x43; // CRC16 checksum

// XModem packet size
const int PACKET_SIZE = 128;

// Response timeout in milliseconds
const int RESPONSE_TIMEOUT = 10000;
const int TRANSMISSION_TIMEOUT = 60000;


#endif //UNTITLED_VARIABLES_H