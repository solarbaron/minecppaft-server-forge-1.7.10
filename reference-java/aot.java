/*
 * Decompiled with CFR 0.152.
 */
import java.util.concurrent.Callable;

class aot
implements Callable {
    final /* synthetic */ aor a;

    aot(aor aor2) {
        this.a = aor2;
    }

    public String a() {
        int n2 = aji.b(this.a.b.a(this.a.c, this.a.d, this.a.e));
        try {
            return String.format("ID #%d (%s // %s)", n2, aji.e(n2).a(), aji.e(n2).getClass().getCanonicalName());
        }
        catch (Throwable throwable) {
            return "ID #" + n2;
        }
    }

    public /* synthetic */ Object call() {
        return this.a();
    }
}

