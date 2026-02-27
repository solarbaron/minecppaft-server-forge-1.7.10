/*
 * Decompiled with CFR 0.152.
 */
import java.util.concurrent.Callable;

class sb
implements Callable {
    final /* synthetic */ sa a;

    sb(sa sa2) {
        this.a = sa2;
    }

    public String a() {
        return sg.b(this.a) + " (" + this.a.getClass().getCanonicalName() + ")";
    }

    public /* synthetic */ Object call() {
        return this.a();
    }
}

