package org.example;

import java.io.*;

public class XModemSender {

    // STX, SOH, EOT and ACK characters
    private static final byte STX = 0x02;
    private static final byte SOH = 0x01;
    private static final byte EOT = 0x04;
    private static final byte ACK = 0x06;

    // XModem packet size
    private static final int PACKET_SIZE = 128;

    // Wait time for ACK response in milliseconds
    private static final int ACK_TIMEOUT = 1000;

    // XModem packet number
    private byte packetNumber = 1;

    // Input and output streams
    private InputStream inputStream;
    private OutputStream outputStream;

    public XModemSender(InputStream inputStream, OutputStream outputStream) {
        this.inputStream = inputStream;
        this.outputStream = outputStream;
    }

    public void sendFile(String fileName) throws IOException {
        File file = new File(fileName);
        FileInputStream fileInputStream = new FileInputStream(file);

        // Send file name
        sendPacket(file.getName().getBytes());

        // Send file size
        sendPacket(String.valueOf(file.length()).getBytes());

        // Send file contents
        byte[] buffer = new byte[PACKET_SIZE];
        int bytesRead = 0;

        while ((bytesRead = fileInputStream.read(buffer)) > 0) {
            sendPacket(buffer, bytesRead);
        }

        // Send end of transmission
        sendPacket(new byte[] { EOT });

        fileInputStream.close();
    }

    private void sendPacket(byte[] data) throws IOException {
        sendPacket(data, data.length);
    }

    private void sendPacket(byte[] data, int length) throws IOException {
        byte[] packet = new byte[PACKET_SIZE + 4];

        if (length == PACKET_SIZE) {
            packet[0] = STX;
        } else {
            packet[0] = SOH;
        }

        packet[1] = packetNumber;
        packet[2] = (byte) (~packetNumber);

        System.arraycopy(data, 0, packet, 3, length);

        byte checksum = 0;
        for (int i = 3; i < PACKET_SIZE + 3; i++) {
            checksum += packet[i];
        }
        packet[PACKET_SIZE + 3] = checksum;

        boolean ackReceived = false;
        while (!ackReceived) {
            outputStream.write(packet);
            outputStream.flush();

            byte[] response = new byte[1];
            long startTime = System.currentTimeMillis();

            while (System.currentTimeMillis() - startTime < ACK_TIMEOUT) {
                if (inputStream.available() > 0) {
                    inputStream.read(response);
                    if (response[0] == ACK) {
                        ackReceived = true;
                        packetNumber++;
                        break;
                    }
                }
            }

            if (!ackReceived) {
                // Resend packet
            }
        }
    }
}
