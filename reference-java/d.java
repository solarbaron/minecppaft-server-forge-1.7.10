/*
 * Decompiled with CFR 0.152.
 */
import java.util.concurrent.Callable;

class d
implements Callable {
    final /* synthetic */ b a;

    d(b b2) {
        this.a = b2;
    }

    public String a() {
        return System.getProperty("os.name") + " (" + System.getProperty("os.arch") + ") version " + System.getProperty("os.version");
    }

    public /* synthetic */ Object call() {
        return this.a();
    }
}

