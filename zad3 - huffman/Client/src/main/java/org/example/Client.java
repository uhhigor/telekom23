package org.example;

import java.io.*;
import java.net.Socket;
import java.nio.file.Files;
import java.nio.file.Paths;

public class Client {
    private DataOutputStream dataOutputStream;
    private DataInputStream dataInputStream;
    private ObjectOutputStream objectOutputStream;
    private Socket socket;

    public void startConnection(int portNumber, String nameFile) throws IOException {
        socket = new Socket("localhost", portNumber);
        dataInputStream = new DataInputStream(socket.getInputStream());
        dataOutputStream = new DataOutputStream(socket.getOutputStream());
        readFile(nameFile);
        stopConnection();
    }

    public void readFile(String nameFile) {
        try {
            File file = new File(nameFile);
            String str = new String(Files.readAllBytes(Paths.get(nameFile)));
            int len = str.length();
            try (FileReader fileReader = new FileReader(file)) {
                int character, i = 0;
                char [] charArray = new char[len];
                while ((character = fileReader.read()) != -1) {
                    char c = (char) character;
                    charArray[i] = c;
                    i++;
                }
                Huffman huffman = new Huffman();
                dataOutputStream.writeUTF(huffman.huffmanTree(charArray));

                Node root = huffman.createHuffmanTree(charArray);
                objectOutputStream = new ObjectOutputStream(socket.getOutputStream());
                objectOutputStream.writeObject(root);
                objectOutputStream.flush();
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
        objectOutputStream.close();
        socket.close();
    }
}
