package org.example;

import gnu.io.*;

public class SerialPorts {
    private static final String PORT_NAME = "/dev/ttyUSB0"; // nazwa portu szeregowego
    private static final int BAUD_RATE = 9600; // prędkość transmisji w baudach
    private static final int DATA_BITS = SerialPort.DATABITS_8; // liczba bitów danych
    private static final int STOP_BITS = SerialPort.STOPBITS_1; // liczba bitów stopu
    private static final int PARITY = SerialPort.PARITY_NONE; // bit parzystości

    public SerialPorts() {
        SerialPort serialPort = null;
        try {
            CommPortIdentifier portIdentifier = CommPortIdentifier.getPortIdentifier(PORT_NAME);
            if (portIdentifier.isCurrentlyOwned()) {
                System.err.println("Port " + PORT_NAME + " is currently in use");
                return;
            }
            CommPort commPort = portIdentifier.open(SerialPort.class.getName(), 2000);
            if (!(commPort instanceof SerialPort)) {
                System.err.println("Port " + PORT_NAME + " is not a serial port");
                return;
            }
            serialPort = (SerialPort) commPort;
            serialPort.setSerialPortParams(BAUD_RATE, DATA_BITS, STOP_BITS, PARITY);
            System.out.println("Serial port " + PORT_NAME + " is ready for use");
        } catch (NoSuchPortException | PortInUseException | UnsupportedCommOperationException ex) {
            System.err.println("Error while configuring serial port: " + ex.getMessage());
        } finally {
            if (serialPort != null) {
                serialPort.close();
            }
        }
    }
}

