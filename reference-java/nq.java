/*
 * Decompiled with CFR 0.152.
 */
import io.netty.util.concurrent.GenericFutureListener;
import net.minecraft.server.MinecraftServer;

public class nq
implements kh {
    private final MinecraftServer a;
    private final ej b;

    public nq(MinecraftServer minecraftServer, ej ej2) {
        this.a = minecraftServer;
        this.b = ej2;
    }

    @Override
    public void a(fj fj2) {
    }

    @Override
    public void a(eo eo2, eo eo3) {
        if (eo3 != eo.c) {
            throw new UnsupportedOperationException("Unexpected change in protocol to " + (Object)((Object)eo3));
        }
    }

    @Override
    public void a() {
    }

    @Override
    public void a(kj kj2) {
        this.b.a(new ka(this.a.ay()), new GenericFutureListener[0]);
    }

    @Override
    public void a(ki ki2) {
        this.b.a(new jz(ki2.c()), new GenericFutureListener[0]);
    }
}

