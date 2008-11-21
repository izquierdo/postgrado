public class kaldewaij413_jml {

    public static void main(String args[]) {
        int A = 9;
        int B = 15;

        new kaldewaij413_jml().algoritmo(A, B);
    }

    void algoritmo(final int A, final int B) {
        //@ assert A > 0 && B > 0;

        int x;

        x = A;

        //@ maintaining 1 <= x && x <= A*B && x % A == 0 && (\forall int i ; 1 <= i && i < x && i % A == 0 ; i % B != 0);
        //@ decreasing A*B-x;
        while (x % B != 0) {
            x = x + A;
        }

        //@ assert x == (\min int i ; 1 <= i && i % A == 0 && i % B == 0 ; i);
    }
}
