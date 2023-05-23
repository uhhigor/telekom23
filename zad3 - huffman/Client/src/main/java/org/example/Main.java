package org.example;

import java.io.File;
import java.io.IOException;
import java.util.Scanner;

public class Main {
    public static void main(String[] args) throws IOException {
        Scanner scanner = new Scanner(System.in);

        System.out.print("Enter port number: ");
        int portNumber = scanner.nextInt();

        System.out.print("Enter file name: ");
        String fileName = scanner.next();

        File file = new File(fileName);
        String absolutePath = file.getAbsolutePath();
        System.out.println(absolutePath);

        Client client = new Client();
        client.startConnection(portNumber, absolutePath);
    }
}