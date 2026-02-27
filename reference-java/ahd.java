/*
 * Decompiled with CFR 0.152.
 */
import java.util.concurrent.Callable;

class ahd
implements Callable {
    final /* synthetic */ aji a;
    final /* synthetic */ ahb b;

    ahd(ahb ahb2, aji aji2) {
        this.b = ahb2;
        this.a = aji2;
    }

    public String a() {
        try {
            return String.format("ID #%d (%s // %s)", aji.b(this.a), this.a.a(), this.a.getClass().getCanonicalName());
        }
        catch (Throwable throwable) {
            return "ID #" + aji.b(this.a);
        }
    }

    public /* synthetic */ Object call() {
        return this.a();
    }
}

