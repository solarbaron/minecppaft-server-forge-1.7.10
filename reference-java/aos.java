/*
 * Decompiled with CFR 0.152.
 */
import java.util.concurrent.Callable;

class aos
implements Callable {
    final /* synthetic */ aor a;

    aos(aor aor2) {
        this.a = aor2;
    }

    public String a() {
        return (String)aor.v().get(this.a.getClass()) + " // " + this.a.getClass().getCanonicalName();
    }

    public /* synthetic */ Object call() {
        return this.a();
    }
}

