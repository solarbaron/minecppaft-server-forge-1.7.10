/*
 * Decompiled with CFR 0.152.
 */
import java.util.concurrent.Callable;

class nf
implements Callable {
    final /* synthetic */ ej a;
    final /* synthetic */ nc b;

    nf(nc nc2, ej ej2) {
        this.b = nc2;
        this.a = ej2;
    }

    public String a() {
        return this.a.toString();
    }

    public /* synthetic */ Object call() {
        return this.a();
    }
}

