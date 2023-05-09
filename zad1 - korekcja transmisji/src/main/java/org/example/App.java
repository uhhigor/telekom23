package org.example;

import java.io.*;

public class App 
{
    static Console c;
    public static void main( String[] args )
    {
        System.out.println("1 ) Zakoduj plik");
        System.out.println("2 ) Odkoduj plik");
        System.out.println("3 ) Zakończ");

        c = System.console();
        if (c == null) {
            System.out.println("Brak konsoli");
            return;
        }
            String choose = c.readLine();
            switch (choose) {
                case "1" -> {
                    System.out.println("Podaj ścieżkę do pliku: ");
                    fileEncoding.encodeFile(c.readLine());
                }
                case "2" -> {
                    System.out.println("Podaj ścieżkę do pliku: ");
                    fileEncoding.decodeFile(c.readLine());
                }
                default -> {
                }
        }
    }
}
