package org.example;

import java.io.File;
import java.io.IOException;
import java.util.Scanner;

public class Main {
    public static void main(String[] args) throws IOException {
        Scanner scanner = new Scanner(System.in);

        System.out.println("Enter port number: ");
        int portNumber = scanner.nextInt();

//        System.out.println("Enter file name: ");
//        String fileName = scanner.next();

//        File file = new File(fileName);
//        String absolutePath = file.getAbsolutePath();
//        System.out.println(absolutePath);

        Server server = new Server();
        server.startConnection(portNumber);

    }
}