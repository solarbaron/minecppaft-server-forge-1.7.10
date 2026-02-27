/*
 * Decompiled with CFR 0.152.
 */
import io.netty.channel.ChannelFutureListener;
import io.netty.util.concurrent.GenericFutureListener;

class ek
implements Runnable {
    final /* synthetic */ eo a;
    final /* synthetic */ eo b;
    final /* synthetic */ ft c;
    final /* synthetic */ GenericFutureListener[] d;
    final /* synthetic */ ej e;

    ek(ej ej2, eo eo2, eo eo3, ft ft2, GenericFutureListener[] genericFutureListenerArray) {
        this.e = ej2;
        this.a = eo2;
        this.b = eo3;
        this.c = ft2;
        this.d = genericFutureListenerArray;
    }

    @Override
    public void run() {
        if (this.a != this.b) {
            this.e.a(this.a);
        }
        ej.a(this.e).writeAndFlush(this.c).addListeners(this.d).addListener(ChannelFutureListener.FIRE_EXCEPTION_ON_FAILURE);
    }
}

