package org.example;

import java.io.*;

public class XModemReceiver {

    // XModem control characters
    private static final byte SOH = 0x01; // Start Of Header
    private static final byte EOT = 0x04; // End Of Transmission
    private static final byte ACK = 0x06; // Acknowledge
    private static final byte NAK = 0x15; // Negative Acknowledge

    // XModem packet size
    private static final int PACKET_SIZE = 128;

    // Input and output streams
    private final InputStream inputStream;
    private final OutputStream outputStream;

    public XModemReceiver(InputStream inputStream, OutputStream outputStream) {
        this.inputStream = inputStream;
        this.outputStream = outputStream;
    }

    public void receiveFile(String fileName) throws IOException {
        // Open file for writing
        File file = new File(fileName);
        FileOutputStream fileOutputStream = new FileOutputStream(file);

        // Send NAK to sender
        outputStream.write(NAK);
        outputStream.flush();

        // Receive file contents
        int blockNumber = 1;
        boolean eofReceived = false;
        while (!eofReceived) {
            // Receive packet
            byte[] packet = receivePacket(blockNumber);

            // Process packet
            if (packet == null) {
                // Timeout waiting for packet
                throw new IOException("Timeout waiting for packet from sender");
            } else if (packet.length == 0) {
                // End of file received
                eofReceived = true;
            } else if (packet[1] == blockNumber) {
                // Valid packet received
                fileOutputStream.write(packet, 3, PACKET_SIZE);
                blockNumber++;
                outputStream.write(ACK);
                outputStream.flush();
            } else {
                // Invalid block number received
                outputStream.write(NAK);
                outputStream.flush();
            }
        }

        // Send ACK to sender
        outputStream.write(ACK);
        outputStream.flush();

        // Close file output stream
        fileOutputStream.close();
    }

    private byte[] receivePacket(int blockNumber) throws IOException {
        // Receive packet
        byte[] packet = new byte[PACKET_SIZE + 3];
        int bytesRead;
        long startTime = System.currentTimeMillis();
        while (System.currentTimeMillis() - startTime < XModemSender.RESPONSE_TIMEOUT) {
            if (inputStream.available() >= PACKET_SIZE + 3) {
                bytesRead = inputStream.read(packet);
                if (bytesRead == PACKET_SIZE + 3 && packet[0] == SOH && packet[1] == blockNumber && packet[2] == ~blockNumber) {
                    // Valid packet received
                    byte checksum = 0;
                    for (int i = 0; i < PACKET_SIZE; i++) {
                        checksum += packet[i + 3];
                    }
                    if (checksum == packet[PACKET_SIZE + 3 - 1]) {
                        // Checksum is valid
                        return packet;
                    } else {
                        // Checksum is invalid
                        outputStream.write(NAK);
                        outputStream.flush();
                    }
                } else if (bytesRead == 1 && packet[0] == EOT) {
                    // End of file received
                    return new byte[0];
                } else {
                    // Invalid packet received
                    outputStream.write(NAK);
                    outputStream.flush();
                }
            }
        }
        return null;
    }
}
