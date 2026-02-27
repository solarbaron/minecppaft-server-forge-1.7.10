/*
 * Decompiled with CFR 0.152.
 */
import java.util.concurrent.Callable;

class e
implements Callable {
    final /* synthetic */ b a;

    e(b b2) {
        this.a = b2;
    }

    public String a() {
        return System.getProperty("java.version") + ", " + System.getProperty("java.vendor");
    }

    public /* synthetic */ Object call() {
        return this.a();
    }
}

