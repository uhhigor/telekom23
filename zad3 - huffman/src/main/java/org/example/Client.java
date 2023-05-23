package org.example;

import java.io.*;
import java.net.Socket;
import java.util.Scanner;

public class Client {
    private DataOutputStream dataOutputStream;
    private DataInputStream dataInputStream;
    private Socket socket;

    public void startConnection(int portNumber, String nameFile) throws IOException {
        socket = new Socket("localhost", portNumber);
        dataInputStream = new DataInputStream(socket.getInputStream());
        dataOutputStream = new DataOutputStream(socket.getOutputStream());
        readFile(nameFile);
        stopConnection();
    }

    public void readFile(String nameFile)  {
        try {
            File file = new File(nameFile);
            Scanner scanner = new Scanner(file);
            while(scanner.hasNextLine()) {
                String data = scanner.nextLine();
                dataOutputStream.writeUTF(data);
            }
        } catch (FileNotFoundException e) {
            System.out.println("Can't open the file");
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    public void stopConnection() throws IOException {
        dataInputStream.close();
        dataOutputStream.close();
        socket.close();
    }
}
