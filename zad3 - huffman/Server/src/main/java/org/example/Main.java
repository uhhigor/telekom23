package org.example;

import java.util.Scanner;

public class Main {
    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);

        System.out.print("Enter port number: ");
        int portNumber = scanner.nextInt();

        Server server = new Server();
        server.startConnection(portNumber);
    }
}