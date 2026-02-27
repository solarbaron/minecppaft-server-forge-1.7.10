/*
 * Decompiled with CFR 0.152.
 */
import java.util.concurrent.Callable;

class aza
implements Callable {
    final /* synthetic */ ays a;

    aza(ays ays2) {
        this.a = ays2;
    }

    public String a() {
        return String.format("Rain time: %d (now: %b), thunder time: %d (now: %b)", ays.k(this.a), ays.l(this.a), ays.m(this.a), ays.n(this.a));
    }

    public /* synthetic */ Object call() {
        return this.a();
    }
}

