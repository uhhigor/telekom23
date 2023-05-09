package org.example;

import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class XModemReceiver {

    // STX, SOH, EOT and ACK characters
    private static final byte STX = 0x02;
    private static final byte SOH = 0x01;
    private static final byte EOT = 0x04;
    private static final byte ACK = 0x06;

    // XModem packet size
    private static final int PACKET_SIZE = 128;

    // Wait time for packet in milliseconds
    private static final int PACKET_TIMEOUT = 1000;

    // Input and output streams
    private InputStream inputStream;
    private OutputStream outputStream;

    public XModemReceiver(InputStream inputStream, OutputStream outputStream) {
        this.inputStream = inputStream;
        this.outputStream = outputStream;
    }

    public void receiveFile(String fileName) throws IOException {
        FileOutputStream fileOutputStream = new FileOutputStream(fileName);
        byte[] buffer = new byte[PACKET_SIZE];
        boolean endOfTransmission = false;
        byte expectedPacketNumber = 1;

        while (!endOfTransmission) {
            byte[] packet = readPacket();

            if (packet == null) {
                // Timeout error
                break;
            }

            byte packetNumber = packet[1];
            byte packetNumberComplement = packet[2];

            if (packetNumber != ~packetNumberComplement) {
                // Packet number error
                break;
            }

            if (packetNumber == expectedPacketNumber) {
                // Write data to file
                fileOutputStream.write(packet, 3, PACKET_SIZE);
                expectedPacketNumber++;

                // Send ACK
                outputStream.write(ACK);
                outputStream.flush();
            } else if (packetNumber == 1 && expectedPacketNumber == 0) {
                // XModem-1K block mode
                fileOutputStream.write(buffer, 0, PACKET_SIZE);
                fileOutputStream.write(packet, 3, PACKET_SIZE);
                expectedPacketNumber = 2;

                // Send ACK
                outputStream.write(ACK);
                outputStream.flush();
            } else if (packet[0] == EOT) {
                // End of transmission
                endOfTransmission = true;

                // Send ACK
                outputStream.write(ACK);
                outputStream.flush();
            } else {
                // Unknown packet
            }
        }

        fileOutputStream.close();
    }

    private byte[] readPacket() throws IOException {
        byte[] packet = new byte[PACKET_SIZE + 4];

        long startTime = System.currentTimeMillis();
        while (System.currentTimeMillis() - startTime < PACKET_TIMEOUT) {
            if (inputStream.available() > 0) {
                int bytesRead = inputStream.read(packet);
                if (bytesRead == PACKET_SIZE + 4) {
                    return packet;
                }
            }
        }

        return null;
    }
}

