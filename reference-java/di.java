/*
 * Decompiled with CFR 0.152.
 */
import java.util.concurrent.Callable;

class di
implements Callable {
    final /* synthetic */ String a;
    final /* synthetic */ dh b;

    di(dh dh2, String string) {
        this.b = dh2;
        this.a = string;
    }

    public String a() {
        return dy.a[((dy)dh.a(this.b).get(this.a)).a()];
    }

    public /* synthetic */ Object call() {
        return this.a();
    }
}

