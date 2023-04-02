package org.example;

import zadanie1.byteMessage;

public class App 
{
    public static void main( String[] args )
    {
        byte message = (byte) 0xC1; //11000001
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


    }
}
