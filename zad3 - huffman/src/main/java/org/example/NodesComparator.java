package org.example;

import java.util.Comparator;

public class NodesComparator implements Comparator<Node> {

    @Override
    public int compare(Node o1, Node o2) {
        return o1.frequency - o2.frequency;
    }
}
