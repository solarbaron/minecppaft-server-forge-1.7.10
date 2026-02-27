/*
 * Decompiled with CFR 0.152.
 */
import io.netty.util.concurrent.Future;
import io.netty.util.concurrent.GenericFutureListener;

class ni
implements GenericFutureListener {
    final /* synthetic */ fq a;
    final /* synthetic */ nh b;

    ni(nh nh2, fq fq2) {
        this.b = nh2;
        this.a = fq2;
    }

    public void operationComplete(Future future) {
        this.b.a.a(this.a);
    }
}

