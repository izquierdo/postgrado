public class kaldewaij435_jml {

    static int A[] = new int[] {-42,-3,8,-14,-6,-43,-123,-4,-439,3,2,5,0,-12,2,4,34,-50,-9,-23,-2,-18,-104,-22,-40,-40,-3,-34,0,-11,-11,-3,-43,-99,-66};

    public static void main(String args[]) {
        new kaldewaij435_jml().algoritmo(A.length, A);
    }

    /* defino este metodo para facilitar la escritura de condiciones */
    //@ ensures \result == ((\num_of int i ; p<=i && i<q ; A[i] > 0) - (\num_of int i ; p<=i && i<q ; A[i] < 0));
    /*@ pure @*/
    int credit(int p, int q) {
        int c=0;

        for (int i = p; i < q; i++) {
            if (A[i] > 0)
                c++;

            if (A[i] < 0)
                c--;
        }

        return c;
    }

    void algoritmo(final int N, final int[] A) {
        //@ assert N >= 0;

        int x = 0, y = 0, m = 0, n = 0, s = 0, e = 0;

        //@ maintaining m == (\max int p ; 0 <= p && p <= n ; (\max int q ; p <= q && q <= n ; credit(p, q))) &&
        //@             s == (\max int p ; 0 <= p && p <= n ; credit(p, n)) &&
        //@             0<=n && n<=N &&
        //@             m == credit(x, y);
        //@ decreasing N-n;
        while (n != N) {
            if (A[n] < 0) {
                if (s-1 < 0) {
                    s = 0;
                    e = n + 1;
                } else if (s-1 >= 0) {
                    s = s -1;
                }
            } else if (A[n] == 0) {
            } else if (A[n] > 0) {
                s = s + 1;
            }

            if (m <= s) {
                m = s;
                x = e;
                y = n + 1;
            } else if (m > s) {
            }

            n = n + 1;
        }

        //@ assert m == (\max int p ; 0 <= p && p <= N ; (\max int q ; p <= q && q <= N ; credit(p, q))) &&
        //@        m == credit(x,y);
    }
}
