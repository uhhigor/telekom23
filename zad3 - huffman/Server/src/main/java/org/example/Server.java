package org.example;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.Socket;
import java.net.ServerSocket;

public class Server {
    private DataOutputStream dataOutputStream;
    private DataInputStream dataInputStream;
    private ServerSocket serverSocket;
    private Socket clientSocket;

    public void startConnection(int portNumber) throws IOException {
        ServerSocket serverSocket = new ServerSocket(portNumber);
        clientSocket = serverSocket.accept();
        dataInputStream = new DataInputStream(clientSocket.getInputStream());
        dataOutputStream = new DataOutputStream(clientSocket.getOutputStream());
        System.out.print("Encoded message: ");
        String message = dataInputStream.readUTF();
        System.out.println(message);
        stopConnection();
    }
    public void stopConnection() throws IOException {
        dataInputStream.close();
        dataOutputStream.close();
        clientSocket.close();
    }
}
