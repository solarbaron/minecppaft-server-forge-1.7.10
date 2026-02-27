/*
 * Decompiled with CFR 0.152.
 */
import java.util.concurrent.Callable;

class f
implements Callable {
    final /* synthetic */ b a;

    f(b b2) {
        this.a = b2;
    }

    public String a() {
        return System.getProperty("java.vm.name") + " (" + System.getProperty("java.vm.info") + "), " + System.getProperty("java.vm.vendor");
    }

    public /* synthetic */ Object call() {
        return this.a();
    }
}

