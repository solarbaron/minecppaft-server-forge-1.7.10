/*
 * Decompiled with CFR 0.152.
 */
import java.util.concurrent.Callable;

class ahe
implements Callable {
    final /* synthetic */ ahb a;

    ahe(ahb ahb2) {
        this.a = ahb2;
    }

    public String a() {
        return this.a.h.size() + " total; " + this.a.h.toString();
    }

    public /* synthetic */ Object call() {
        return this.a();
    }
}

