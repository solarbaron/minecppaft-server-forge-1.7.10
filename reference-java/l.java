/*
 * Decompiled with CFR 0.152.
 */
import java.util.concurrent.Callable;

final class l
implements Callable {
    final /* synthetic */ int a;
    final /* synthetic */ aji b;

    l(int n2, aji aji2) {
        this.a = n2;
        this.b = aji2;
    }

    public String a() {
        try {
            return String.format("ID #%d (%s // %s)", this.a, this.b.a(), this.b.getClass().getCanonicalName());
        }
        catch (Throwable throwable) {
            return "ID #" + this.a;
        }
    }

    public /* synthetic */ Object call() {
        return this.a();
    }
}

