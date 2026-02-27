/*
 * Decompiled with CFR 0.152.
 */
import java.util.concurrent.Callable;

class apy
implements Callable {
    final /* synthetic */ int a;
    final /* synthetic */ int b;
    final /* synthetic */ int c;
    final /* synthetic */ apx d;

    apy(apx apx2, int n2, int n3, int n4) {
        this.d = apx2;
        this.a = n2;
        this.b = n3;
        this.c = n4;
    }

    public String a() {
        return k.a(this.a, this.b, this.c);
    }

    public /* synthetic */ Object call() {
        return this.a();
    }
}

