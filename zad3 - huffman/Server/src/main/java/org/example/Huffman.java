package org.example;

public class Huffman {

    public String decodeData(Node root, String encodedData) {
        StringBuilder decodedString = new StringBuilder();
        Node currentNode = root;

        for (int i = 0; i < encodedData.length(); i++) {
            char bit = encodedData.charAt(i);

            if (bit == '0') {
                currentNode = currentNode.left;
            } else if (bit == '1') {
                currentNode = currentNode.right;
            }

            if (currentNode.left == null && currentNode.right == null) {
                decodedString.append(currentNode.character);
                currentNode = root;
            }
        }
        return decodedString.toString();
    }
}
