package zadanie1;

public class singleByteErrorMessage {

    private final short[] H = new short[] {
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
                }
                System.out.println("Row="+matrixRow+" sum="+sum);
                if(sum % 2 != 0)
                    result+=1;
            }
        return result;
    }
}
