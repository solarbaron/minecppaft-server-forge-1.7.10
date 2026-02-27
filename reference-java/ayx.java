/*
 * Decompiled with CFR 0.152.
 */
import java.util.concurrent.Callable;

class ayx
implements Callable {
    final /* synthetic */ ays a;

    ayx(ays ays2) {
        this.a = ays2;
    }

    public String a() {
        return String.format("%d game time, %d day time", ays.g(this.a), ays.h(this.a));
    }

    public /* synthetic */ Object call() {
        return this.a();
    }
}

