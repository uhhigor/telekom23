package org.example;

import javax.swing.*;
import zadanie1.singleByteErrorMessage;

public class App 
{
    public static void main( String[] args )
    {
        short[] H = new short[] {
                (short) 0xEE8,
                (short) 0xD94,
                (short) 0xB42,
                (short) 0x731,
        };
        short vector = (short) 0xA6;
        System.out.println(Integer.toBinaryString(vector));
        singleByteErrorMessage singleByteErrorMessage = new singleByteErrorMessage();
        vector = singleByteErrorMessage.encoding(H, vector);
        System.out.println(Integer.toBinaryString(vector));
        vector = (short) 0xA76;
        System.out.println(Integer.toBinaryString(vector));
        singleByteErrorMessage.decoding(H, vector);
        System.out.println(Integer.toBinaryString(singleByteErrorMessage.decoding(H, vector)));


        //System.out.println(Integer.toBinaryString(vector));
        /*short message = (short) 0xA046;
        short[] matrix = new short[] {
                (short) 0xF080, // 1 1 1 1 0 0 0 0 | 1 0 0 0 0 0 0 0 //1+1%2=0 /// 01000110
                (short) 0xCC40, // 1 1 0 0 1 1 0 0 | 0 1 0 0 0 0 0 0 //1%2=1
                (short) 0xAA20, // 1 0 1 0 1 0 1 0 | 0 0 1 0 0 0 0 0 //1+1%2=0
                (short) 0x5610, // 0 1 0 1 0 1 1 0 | 0 0 0 1 0 0 0 0 //0
                (short) 0xE908, // 1 1 1 0 1 0 0 1 | 0 0 0 0 1 0 0 0 //1+1=2%2=0
                (short) 0x9504, // 1 0 0 1 0 1 0 1 | 0 0 0 0 0 1 0 0 //1
                (short) 0x7B02, // 0 1 1 1 1 0 1 1 | 0 0 0 0 0 0 1 0 //1
                (short) 0xE701  // 1 1 1 0 0 1 1 1 | 0 0 0 0 0 0 0 1 //2%2=0
        };
        int result = singleByteErrorMessage.HTmul(matrix, message);
        System.out.println(Integer.toBinaryString(0xFFFF & message));
        System.out.println(result);*/



    }
}
