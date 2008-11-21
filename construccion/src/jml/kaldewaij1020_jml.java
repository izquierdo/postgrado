public class kaldewaij1020_jml {

    public static void main(String args[]) {
        int h[] = new int[] {-1,2,-3,-4,5,6,7,-2,-8,10};
        new kaldewaij1020_jml().algoritmo(10, h);
    }

    void algoritmo(final int N, int[] h) {
        //@ assert N >= 0;

        int m = 0, n = 0;

        //@ maintaining 0 <= m && m <= n && n <= N && (\forall int i ; 0 <= i && i < m ; h[i] <= 0) && (\forall int i ; m <= i && i < n ; h[i] >= 0);
        //@ decreasing N-n;
        while (n != N) {
            if (h[n] <= 0) {
                {int r = h[m]; h[m] = h[n]; h[n] = r;}
                m = m + 1;
                n = n + 1;
            } else if (h[n] >= 0) {
                n = n+1;
            }
        }

        //@ assert (\forall int i ; 0 <= i && i < m ; h[i] <= 0) && (\forall int i ; m <= i && i < n ; h[i] >= 0);
    }
}
