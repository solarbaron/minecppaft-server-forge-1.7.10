/*
 * Decompiled with CFR 0.152.
 */
import java.util.concurrent.Callable;

class i
implements Callable {
    final /* synthetic */ b a;

    i(b b2) {
        this.a = b2;
    }

    public String a() {
        int n2 = 0;
        int n3 = 56 * n2;
        int n4 = n3 / 1024 / 1024;
        int n5 = 0;
        int n6 = 56 * n5;
        int n7 = n6 / 1024 / 1024;
        return n2 + " (" + n3 + " bytes; " + n4 + " MB) allocated, " + n5 + " (" + n6 + " bytes; " + n7 + " MB) used";
    }

    public /* synthetic */ Object call() {
        return this.a();
    }
}

