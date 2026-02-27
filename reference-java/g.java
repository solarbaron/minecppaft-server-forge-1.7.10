/*
 * Decompiled with CFR 0.152.
 */
import java.util.concurrent.Callable;

class g
implements Callable {
    final /* synthetic */ b a;

    g(b b2) {
        this.a = b2;
    }

    public String a() {
        Runtime runtime = Runtime.getRuntime();
        long l2 = runtime.maxMemory();
        long l3 = runtime.totalMemory();
        long l4 = runtime.freeMemory();
        long l5 = l2 / 1024L / 1024L;
        long l6 = l3 / 1024L / 1024L;
        long l7 = l4 / 1024L / 1024L;
        return l4 + " bytes (" + l7 + " MB) / " + l3 + " bytes (" + l6 + " MB) up to " + l2 + " bytes (" + l5 + " MB)";
    }

    public /* synthetic */ Object call() {
        return this.a();
    }
}

