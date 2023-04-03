package org.example;

import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

public class fileEncoding {
    public static void encodeFile(String fileUrl) {
        byte[] fileBytes;
        try (FileInputStream inputStream = new FileInputStream(fileUrl)) {
            fileBytes = inputStream.readAllBytes();
        } catch (IOException e) {
            System.out.println("Błąd podczas odczytu pliku.");
            return;
        }

        short[] encodedFileShorts = new short[fileBytes.length];
        for(int i = 0; i < fileBytes.length; i++)
            encodedFileShorts[i] = byteMessage.encode(fileBytes[i]);

        byte[] encodedFileBytes = new byte[encodedFileShorts.length * 2];
        ByteBuffer buffer = ByteBuffer.wrap(encodedFileBytes);

        for (short value : encodedFileShorts)
            buffer.putShort(value);

        try (FileOutputStream outputStream = new FileOutputStream(fileUrl)) {
            outputStream.write(encodedFileBytes);
        } catch (IOException e) {
            System.out.println("Błąd podczas zapisu pliku.");
        }
    }

    public static void decodeFile(String fileUrl) {
        byte[] encodedFileBytes;
        try (FileInputStream inputStream = new FileInputStream(fileUrl)) {
            encodedFileBytes = inputStream.readAllBytes();
        } catch (IOException e) {
            System.out.println("Błąd podczas odczytu pliku.");
            return;
        }

        short[] encodedFileShorts = new short[encodedFileBytes.length / 2];
        ByteBuffer buffer = ByteBuffer.wrap(encodedFileBytes);

        for (int i = 0; i < encodedFileShorts.length; i++)
            encodedFileShorts[i] = buffer.getShort();

        byte[] decodedFileBytes = new byte[encodedFileShorts.length];
        for(int i = 0; i < decodedFileBytes.length; i++)
            decodedFileBytes[i] = byteMessage.decode(encodedFileShorts[i]);

        try (FileOutputStream outputStream = new FileOutputStream(fileUrl)) {
            outputStream.write(decodedFileBytes);
        } catch (IOException e) {
            System.out.println("Błąd podczas zapisu pliku.");
        }
    }
}
