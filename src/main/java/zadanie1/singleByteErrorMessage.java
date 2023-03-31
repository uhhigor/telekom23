package zadanie1;

public class singleByteErrorMessage {

   private final short[] H = new short[] {
           (short) 0xEE8, // 1,1,1,0,1,1,1,0, 1,0,0,0
           (short) 0xD94, // 1,1,0,1,1,0,0,1, 0,1,0,0
           (short) 0xB42, // 1,0,1,1,0,1,0,0, 0,0,1,0
           (short) 0x731, // 0,1,1,1,0,0,1,1, 0,0,0,1
   };

   public short encoding(short[] matrix, short message) {
       int sum;
       short codeWord;
       byte parityBits = 0;
       for(int rows = 0; rows < 4; rows++) {
           sum = 0;
          for(int column = 0; column < 8; column++) {
             sum += (matrix[rows] >> (11 - column) & 1) * (message >> (7 - column) & 1);
           }
          if(sum % 2 != 0) {
              parityBits |= 0x1 << (3 - rows);
           }
       }
       codeWord = (short) (message << 4 | parityBits);
       return codeWord;
   }

    public short decoding(short[] matrix, short codeWord) {
       short result = (short) ((codeWord >> 4) & 0xFF);
       short errorMatrix = errorMatrix(matrix, codeWord);

       return result;
    }

    public short errorMatrix(short [] matrix, short codeWord) {
       short errorVector = 0;
       int countSetBits = 0;
       for (int i = 0; i < 4; i++) {
           for(int j = 0; j < 12; j++) {
               countSetBits += ((matrix[i] >> (11 - j) & 1)) & ((codeWord >> (11 - j) & 1));
           }
           int isEven = countSetBits % 2;
           if (isEven != 0) {
               errorVector |= 0x1 << (3 - i);
               //System.out.println(Integer.toBinaryString(errorVector));
           }
       }
       return errorVector;
    }

    /*private final short[] H = new short[] {
            (short) 0xF080, // 1 1 1 1 0 0 0 0 | 1 0 0 0 0 0 0 0
            (short) 0xCC40, // 1 1 0 0 1 1 0 0 | 0 1 0 0 0 0 0 0
            (short) 0xAA20, // 1 0 1 0 1 0 1 0 | 0 0 1 0 0 0 0 0
            (short) 0x5610, // 0 1 0 1 0 1 1 0 | 0 0 0 1 0 0 0 0
            (short) 0xE908, // 1 1 1 0 1 0 0 1 | 0 0 0 0 1 0 0 0
            (short) 0x9504, // 1 0 0 1 0 1 0 1 | 0 0 0 0 0 1 0 0
            (short) 0x7B02, // 0 1 1 1 1 0 1 1 | 0 0 0 0 0 0 1 0
            (short) 0xE701  // 1 1 1 0 0 1 1 1 | 0 0 0 0 0 0 0 1
    };

    private short T;

    private byte E = (byte) 0x0;

    public singleByteErrorMessage(short message) {
        this.T = message;
    }

    //If HTmul() returns 0 the message is correct, else the message is incorrect
    public static int HTmul(short[] matrix, short vector) {
        int result = 0;

            for(int matrixRow = 0; matrixRow < 8; matrixRow++) {
                short sum = 0;
                for(int c = 0; c < 16; c++) { //vector rows and matrix columns
                    sum += (matrix[matrixRow] >> (15-c) & 1) * (vector >> (15-c) & 1);
                    //System.out.println("m " + (matrix[matrixRow] >> (15-c) & 1) + "v " + (vector >> (15-c) & 1));
                }
                if(sum % 2 != 0)
                    result+=1;
                System.out.println("Row="+matrixRow+" sum="+sum);
            }
        return result;
    }*/
}
