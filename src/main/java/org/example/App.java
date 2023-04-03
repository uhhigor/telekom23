package org.example;

import java.io.*;
import java.nio.ByteBuffer;

public class App 
{
    static Console c;
    public static void main( String[] args )
    {
        System.out.println("1 ) Zakoduj plik");
        System.out.println("2 ) Odkoduj plik");
        System.out.println("3 ) Zakończ");

        c = System.console();
        if (c == null) {
            System.out.println("Brak konsoli");
            return;
        }

            String choose = c.readLine();
            switch (choose) {
                case "1" -> {
                    System.out.println("Podaj ścieżkę do pliku: ");
                    encodeFile(c.readLine());
                }
                case "2" -> {
                    System.out.println("Podaj ścieżkę do pliku: ");
                    decodeFile(c.readLine());
                }
                default -> {
                }
        }
        /*byte message = (byte) 0xC1; //11000001
        System.out.println("Wiadomość do przesłania: " + Integer.toBinaryString(0xFF & message));

        short encodedMessage = byteMessage.encode(message);
        System.out.println("Wiadomość przesyłana: " + Integer.toBinaryString(0xFFFF & encodedMessage));

        byte errorVector = byteMessage.errorVector(encodedMessage);
        System.out.println("Wektor błędu: " + Integer.toBinaryString(0xFF & errorVector));

        //transmission error
        encodedMessage |= 1 << 12;
        encodedMessage |= 1 << 11;

        System.out.println("Wiadomość przesyłana z uszkodzeniem: " + Integer.toBinaryString(0xFFFF & encodedMessage));

        errorVector = byteMessage.errorVector(encodedMessage);
        System.out.println("Wektor błędu: " + Integer.toBinaryString(0xFF & errorVector));

        byte decodedMessage = byteMessage.decode(encodedMessage);
        System.out.println("Wiadomość odebrana i poprawiona: " + Integer.toBinaryString(0xFF & decodedMessage));

*/
    }

    private static void encodeFile(String fileUrl) {
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

    private static void decodeFile(String fileUrl) {
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
