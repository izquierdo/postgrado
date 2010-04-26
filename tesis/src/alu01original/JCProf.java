
class JCProf {
    static {
        System.loadLibrary("jcprof");
    }

    native static long getCpuTime();
}
