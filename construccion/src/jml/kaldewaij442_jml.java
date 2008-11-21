public class kaldewaij442_jml {

    public static void main(String args[]) {
        int N = 4;
        int X = 2;
        int f[] = new int[] {1,2,3,4};

        new kaldewaij442_jml().algoritmo(N, X, f);
    }

    void algoritmo(final int N, final int X, final int[] f) {
        //@ assert N >= 0;

        int n, r;

        n = N;
        r = 0;

        //@ maintaining r == (\sum int i ; n <= i && i < N ; f[i] * (\product int j ; 0 <= j && j < i-n ; X));
        //@ decreasing n;
        while (n != 0) {
            r = f[n-1] + X*r;
            n = n-1;
        }

        //@ assert r == (\sum int i ; 0 <= i && i < N ; f[i] * (\product int j ; 0 <= j && j < i ; X));
    }
}
