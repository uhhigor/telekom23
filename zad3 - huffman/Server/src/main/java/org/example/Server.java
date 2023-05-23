package org.example;

import java.io.*;
import java.net.Socket;
import java.net.ServerSocket;

public class Server {
    private DataOutputStream dataOutputStream;
    private DataInputStream dataInputStream;
    private Socket clientSocket;
    private ObjectInputStream objectInputStream;

    public void startConnection(int portNumber) {
        try(ServerSocket serverSocket = new ServerSocket(portNumber)) {
            clientSocket = serverSocket.accept();
            dataInputStream = new DataInputStream(clientSocket.getInputStream());
            dataOutputStream = new DataOutputStream(clientSocket.getOutputStream());
            String encodedMessage = dataInputStream.readUTF();
            writeFile("message", encodedMessage);

            objectInputStream = new ObjectInputStream(clientSocket.getInputStream());
            Node root = (Node) objectInputStream.readObject();

            Huffman huffman = new Huffman();
            String decodedMessage = huffman.decodeData(root, encodedMessage);
            writeFile("decoded", decodedMessage);
            stopConnection();
        } catch (Exception e){
            System.out.println(e.toString());
        }
    }

    public void writeFile(String nameFile, String message){
        try {
            File file = new File(nameFile);
            String absolutePath = file.getAbsolutePath();
            try(FileWriter fileWriter = new FileWriter(absolutePath)) {
                fileWriter.write(message);
            }
        } catch (FileNotFoundException e) {
            System.out.println("Can't open the file");
        }catch (IOException e) {
            e.printStackTrace();
        }
    }

    public void stopConnection() throws IOException {
        dataInputStream.close();
        dataOutputStream.close();
        objectInputStream.close();
        clientSocket.close();
    }
}
