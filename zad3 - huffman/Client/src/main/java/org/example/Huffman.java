package org.example;

import java.util.HashMap;
import java.util.Map;
import java.util.PriorityQueue;

public class Huffman {

    public Node createHuffmanTree(char [] charArray) {
        if(charArray.length == 0)
            return null;

        //sign frequency
        Map<Character, Integer> freq = new HashMap<>();
        for(char c : charArray) {
            freq.put(c, freq.getOrDefault(c, 0) + 1);
        }
        int n = freq.size();
        //sign priority (to create Huffman tree)
        PriorityQueue<Node> queue = new PriorityQueue<>(n, new NodesComparator());

        for (Map.Entry<Character, Integer> entry : freq.entrySet()) {
            Character character = entry.getKey();
            Integer frequency = entry.getValue();

            Node node = new Node();
            node.character = character;
            node.frequency = frequency;

            node.left = null;
            node.right = null;

            queue.add(node);
        }

        Node root = null;

        while (queue.size() > 1) {
            Node x = queue.peek();
            queue.poll();

            Node y = queue.peek();
            queue.poll();

            Node newNode = new Node();
            newNode.frequency = x.frequency + y.frequency;
            newNode.character = '-';

            newNode.left = x;
            newNode.right = y;

            root = newNode;
            queue.add(newNode);
        }
        return root;
    }

    public String huffmanTree(char [] charArray) {
        Node root = createHuffmanTree(charArray);
        //store encoded sign pattern
        Map<Character, String> encodedData = new HashMap<>();
        System.out.println("Code directory: ");
        encodeData(root, "", encodedData);

        StringBuilder stringBuilder = new StringBuilder();
        for(char c : charArray) {
            stringBuilder.append(encodedData.get(c));
        }
        System.out.println("Encoded message: " + stringBuilder);
        return stringBuilder.toString();
    }

    public void encodeData(Node root, String code, Map<Character, String> encodedData) {
        if (root.left == null && root.right == null) {
            encodedData.put(root.character, code.length() > 0 ? code : "1");
            if (root.character == ' ') {
                System.out.println("Space:" + code);
            } else {
                System.out.println(root.character + ":" + code);
            }
            return;
        }
        encodeData(root.left, code + "0", encodedData);
        encodeData(root.right, code + "1", encodedData);
    }
}

