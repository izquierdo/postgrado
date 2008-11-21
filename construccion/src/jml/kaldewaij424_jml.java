public class kaldewaij424_jml {

    public static void main(String args[]) {
        int f[] = new int[] {1,2,3,4,5,6,7,8,9,0};
        new kaldewaij424_jml().algoritmo(10, 10, f);
    }

    void algoritmo(final int N, final int X, final int[] f) {
        //@ assert N >= 0;
        int r = 0, n = 0, x = 1;

        //@ maintaining r == (\sum int i ; 0<=i && i<n ; f[i] * (\product int j ; 0<=j && j<i ; X));
        //@ decreasing N-n;
        while (n != N) {
            r = r + f[n]*x;
            x = x*X;
            n = n + 1;
        }

        //@ assert r == (\sum int i ; 0<=i && i<N ; f[i] * (\product int j ; 0<=j && j<i ; X+1));
        //@ assert (\forall int i ; 0 <=i && i<f.length ; f[i] == \old(f[i]));
    }
}
