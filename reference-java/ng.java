/*
 * Decompiled with CFR 0.152.
 */
import io.netty.util.concurrent.Future;
import io.netty.util.concurrent.GenericFutureListener;

class ng
implements GenericFutureListener {
    final /* synthetic */ ej a;
    final /* synthetic */ fq b;
    final /* synthetic */ nc c;

    ng(nc nc2, ej ej2, fq fq2) {
        this.c = nc2;
        this.a = ej2;
        this.b = fq2;
    }

    public void operationComplete(Future future) {
        this.a.a(this.b);
    }
}

