package zadanie1;

public class byteMessage {

   private static final short[] encodingMatrix = new short[] {
           (short) 0b1111000010000000, // 1 1 1 1 0 0 0 0 | 1 0 0 0 0 0 0 0
           (short) 0b1100110001000000, // 1 1 0 0 1 1 0 0 | 0 1 0 0 0 0 0 0
           (short) 0b1010101000100000, // 1 0 1 0 1 0 1 0 | 0 0 1 0 0 0 0 0
           (short) 0b0101011000010000, // 0 1 0 1 0 1 1 0 | 0 0 0 1 0 0 0 0
           (short) 0b1110100100001000, // 1 1 1 0 1 0 0 1 | 0 0 0 0 1 0 0 0
           (short) 0b1001010100000100, // 1 0 0 1 0 1 0 1 | 0 0 0 0 0 1 0 0
           (short) 0b0111101100000010, // 0 1 1 1 1 0 1 1 | 0 0 0 0 0 0 1 0
           (short) 0b1110011100000001  // 1 1 1 0 0 1 1 1 | 0 0 0 0 0 0 0 1
   };

   private static final byte[] decodingMatrix = new byte[] { //encoding Matrix columns
           (byte) 0b11101101,
           (byte) 0b11011011,
           (byte) 0b10101011,
           (byte) 0b10010110,
           (byte) 0b01101010,
           (byte) 0b01010101,
           (byte) 0b00110011,
           (byte) 0b00001111,
   };

    public static short encode(byte message) {
        byte parityBits = 0;
        for(int mRow = 0; mRow < 8; mRow++) {
            short sum = 0;
            for(int vCol = 0; vCol < 8; vCol++) {
                byte row = (byte) (encodingMatrix[mRow] >> 8);
                sum += ((row >> (7 - vCol)) & 1) * ((message >> (7 - vCol)) & 1);
            }
            if (sum % 2 != 0)
                parityBits |= (1 << (7 - mRow));
        }
        return (short) ((message << 8) | parityBits);
    }

    public static byte decode(short encodedMessage) {
        byte message = (byte) (encodedMessage >> 8);
        byte error = errorVector(encodedMessage);

        if(error != 0)
            //message = correctSingleError(message, error);
            message = correctDoubleError(message, error);

        return message;
    }

    //Correct single error in 8-bit message
    private static byte correctSingleError(byte errorMessage, byte errorVector) {
        for(int i = 0; i < 8; i++) {
            if(errorVector == decodingMatrix[i]) {
                errorMessage ^= (1 << 7-i);
            }
        }
        return errorMessage;
    }

    //Correct double error in 8-bit message
    private static byte correctDoubleError(byte errorMessage, byte errorVector) {
        byte resultMessage = errorMessage;
        for(int i = 0; i < 8; i++) {
            byte columnA = decodingMatrix[i];
            for(int j = 0; j < 8; j++) {
                byte columnB = decodingMatrix[j];
                byte sum = (byte) (columnA ^ columnB);
                if(errorVector == sum) {
                    if(((resultMessage >> 7-i)&1) == ((errorMessage >> 7-i)&1)) {
                        resultMessage ^= (1 << 7 - i);
                        if (i != j)
                            resultMessage ^= (1 << 7 - j);
                    }
                }
            }
        }
        return resultMessage;
    }

    public static byte errorVector(short encodedMessage) {
        byte vector = 0;
        for(int mRow = 0; mRow < 8; mRow++) {
            short sum = 0;
            for(int vCol = 0; vCol < 16; vCol++) {
                short row = encodingMatrix[mRow];
                sum += ((row >> (15 - vCol)) & 1) * ((encodedMessage >> (15 - vCol)) & 1);
            }
            if (sum % 2 != 0)
                vector |= (1 << (7 - mRow));
        }
        return vector;
    }
}
