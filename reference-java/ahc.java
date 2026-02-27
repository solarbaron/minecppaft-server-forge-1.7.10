/*
 * Decompiled with CFR 0.152.
 */
import java.util.concurrent.Callable;

class ahc
implements Callable {
    final /* synthetic */ int a;
    final /* synthetic */ int b;
    final /* synthetic */ ahb c;

    ahc(ahb ahb2, int n2, int n3) {
        this.c = ahb2;
        this.a = n2;
        this.b = n3;
    }

    public String a() {
        return k.a(this.a, 0, this.b);
    }

    public /* synthetic */ Object call() {
        return this.a();
    }
}

