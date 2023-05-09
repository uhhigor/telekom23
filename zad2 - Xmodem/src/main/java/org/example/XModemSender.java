package org.example;

import java.io.*;

public class XModemSender {

    // XModem control characters
    private static final byte SOH = 0x01; // Start Of Header
    private static final byte EOT = 0x04; // End Of Transmission
    private static final byte ACK = 0x06; // Acknowledge
    private static final byte NAK = 0x15; // Negative Acknowledge

    // XModem packet size
    private static final int PACKET_SIZE = 128;

    // Response timeout in milliseconds
    public static final int RESPONSE_TIMEOUT = 10000;

    // Input and output streams
    private final InputStream inputStream;
    private final OutputStream outputStream;

    public XModemSender(InputStream inputStream, OutputStream outputStream) {
        this.inputStream = inputStream;
        this.outputStream = outputStream;
    }

    public void sendFile(String fileName) throws IOException {
        // Open file for reading
        File file = new File(fileName);
        FileInputStream fileInputStream = new FileInputStream(file);

        // Wait for NAK from receiver
        boolean nakReceived = false;
        long startTime = System.currentTimeMillis();
        while (System.currentTimeMillis() - startTime < 60000) {
            if (inputStream.available() > 0) {
                int b = inputStream.read();
                if (b == NAK) {
                    nakReceived = true;
                    break;
                }
            }
            try {
                Thread.sleep(10);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        if (!nakReceived) {
            throw new IOException("Timeout waiting for NAK response from receiver");
        }

        // Send file contents
        int blockNumber = 1;
        byte[] packetData = new byte[PACKET_SIZE];
        int bytesRead;
        while ((bytesRead = fileInputStream.read(packetData)) != -1) {
            // Send packet
            sendPacket(packetData, bytesRead, blockNumber);

            // Increment block number
            blockNumber++;

            // Wait for ACK or NAK response
            boolean ackReceived = false;
            boolean nakReceivedAgain = false;
            startTime = System.currentTimeMillis();
            while (System.currentTimeMillis() - startTime < RESPONSE_TIMEOUT) {
                if (inputStream.available() > 0) {
                    int b = inputStream.read();
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
                    throw new IOException("Timeout waiting for ACK/NAK response from receiver");
                }
            }
        }

        // Send EOT
        sendEOT();

        // Wait for ACK response
        boolean eotAckReceived = false;
        startTime = System.currentTimeMillis();
        while (System.currentTimeMillis() - startTime < RESPONSE_TIMEOUT) {
            if (inputStream.available() > 0) {
                int b = inputStream.read();
                if (b == ACK) {
                    eotAckReceived = true;
                    break;
                }
            }
        }
        if (!eotAckReceived) {
            throw new IOException("Timeout waiting for ACK response from receiver after EOT");
        }

        // Close file input stream
        fileInputStream.close();
    }

    private void sendPacket(byte[] data, int length, int blockNumber) throws IOException {
        // Construct packet
        byte[] packet = new byte[PACKET_SIZE + 3];
        packet[0] = SOH; // Start of Header
        packet[1] = (byte) blockNumber;
        packet[2] = (byte) ~blockNumber;
        System.arraycopy(data, 0, packet, 3, length);

        // Calculate checksum
        byte checksum = 0;
        for (int i = 0; i < PACKET_SIZE; i++) {
            checksum += packet[i + 3];
        }
        packet[PACKET_SIZE + 3 - 1] = checksum;

        // Send packet
        outputStream.write(packet);
        outputStream.flush();
    }

    private void sendEOT() throws IOException {
        outputStream.write(EOT);
        outputStream.flush();
    }
}

