package org.example;

import javax.sound.sampled.*;
import java.io.File;
import java.io.IOException;

class AudioConverter {
    private static final int SAMPLE_RATE = 44100;  //Sampling frequency - 44.1 kHz
    private static final int BITS_PER_SAMPLE = 16; //Quantization level - 16 bits
    TargetDataLine line;

    public void recordSound() {
        AudioFormat audioFormat = new AudioFormat(SAMPLE_RATE, BITS_PER_SAMPLE, 1, true, false);
        try {
            line = AudioSystem.getTargetDataLine(audioFormat);
            line.open(audioFormat);
            line.start();

            byte[] buffer = new byte[SAMPLE_RATE * BITS_PER_SAMPLE / 8];

            Thread recordingThread = new Thread(() -> {
                // Record sound from microphone
                line.read(buffer, 0, buffer.length);

                AudioInputStream audioInputStream = new AudioInputStream(line);
                try {
                    AudioSystem.write(audioInputStream, AudioFileFormat.Type.WAVE, new File("sound.wav"));
                } catch (IOException e) {
                    throw new RuntimeException(e);
                }
            });
            recordingThread.start();

        } catch (LineUnavailableException e) {
            e.printStackTrace();
        }
    }

    public void stopRecord() {
        line.stop();
        line.close();
    }

    public void playSound() {
        AudioInputStream audioInputStream;
        SourceDataLine sourceLine;
        try {
            audioInputStream = AudioSystem.getAudioInputStream(new File("sound.wav"));
            AudioFormat sourceFormat = audioInputStream.getFormat();

            DataLine.Info info = new DataLine.Info(SourceDataLine.class, sourceFormat);
            sourceLine = (SourceDataLine) AudioSystem.getLine(info);
            sourceLine.open(sourceFormat);
            sourceLine.start();

            byte[] buffer = new byte[SAMPLE_RATE * BITS_PER_SAMPLE / 8];

            int bytesRead;
            while ((bytesRead = audioInputStream.read(buffer, 0, buffer.length)) != -1) {
                //play sound on speaker
                sourceLine.write(buffer, 0, bytesRead);
            }

            sourceLine.drain();
            sourceLine.stop();
            sourceLine.close();
        } catch (UnsupportedAudioFileException | IOException | LineUnavailableException e) {
            e.printStackTrace();
        }
    }
}