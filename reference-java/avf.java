/*
 * Decompiled with CFR 0.152.
 */
import java.util.concurrent.Callable;

class avf
implements Callable {
    final /* synthetic */ int a;
    final /* synthetic */ int b;
    final /* synthetic */ ave c;

    avf(ave ave2, int n2, int n3) {
        this.c = ave2;
        this.a = n2;
        this.b = n3;
    }

    public String a() {
        return this.c.a(this.a, this.b) ? "True" : "False";
    }

    public /* synthetic */ Object call() {
        return this.a();
    }
}

