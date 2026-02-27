/*
 * Decompiled with CFR 0.152.
 */
import io.netty.util.concurrent.GenericFutureListener;
import net.minecraft.server.MinecraftServer;

public class nl
implements jq {
    private final MinecraftServer a;
    private final ej b;

    public nl(MinecraftServer minecraftServer, ej ej2) {
        this.a = minecraftServer;
        this.b = ej2;
    }

    @Override
    public void a(jp jp2) {
        switch (jp2.c()) {
            case d: {
                this.b.a(eo.d);
                if (jp2.d() > 5) {
                    fq fq2 = new fq("Outdated server! I'm still on 1.7.10");
                    this.b.a(new ju(fq2), new GenericFutureListener[0]);
                    this.b.a(fq2);
                    break;
                }
                if (jp2.d() < 5) {
                    fq fq3 = new fq("Outdated client! Please use 1.7.10");
                    this.b.a(new ju(fq3), new GenericFutureListener[0]);
                    this.b.a(fq3);
                    break;
                }
                this.b.a(new nn(this.a, this.b));
                break;
            }
            case c: {
                this.b.a(eo.c);
                this.b.a(new nq(this.a, this.b));
                break;
            }
            default: {
                throw new UnsupportedOperationException("Invalid intention " + (Object)((Object)jp2.c()));
            }
        }
    }

    @Override
    public void a(fj fj2) {
    }

    @Override
    public void a(eo eo2, eo eo3) {
        if (eo3 != eo.d && eo3 != eo.c) {
            throw new UnsupportedOperationException("Invalid state " + (Object)((Object)eo3));
        }
    }

    @Override
    public void a() {
    }
}

