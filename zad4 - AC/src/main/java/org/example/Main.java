package org.example;

import java.util.Scanner;

public class Main {
    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);

        System.out.println("Enter mode: ");
        System.out.println("1. Record sound");
        System.out.println("2. Play the sound");
        System.out.println("3. Exit program");


        AudioConverter audioConverter = new AudioConverter();

        while (true) {
            System.out.print("Choice: ");
            int mode = scanner.nextInt();

            if(mode == 1) {
                System.out.print("Type y or Y to start recording: ");
                String start = scanner.next();
                if(start.equalsIgnoreCase("y"))
                {
                    System.out.println("Recording has started");
                    audioConverter.recordSound();
                    System.out.print("Type s or S to stop recording: ");
                    String stop = scanner.next();
                    if(stop.equalsIgnoreCase("s")) {
                        audioConverter.stopRecord();
                        System.out.println("Recording has ended");
                    }
                }
            } else if(mode == 2) {
                System.out.println("Playing has started");
                audioConverter.playSound();
            } else {
                break;
            }
        }

    }
}
