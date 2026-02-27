/*
 * Decompiled with CFR 0.152.
 */
import java.util.concurrent.Callable;

class nj
implements Callable {
    final /* synthetic */ ft a;
    final /* synthetic */ nh b;

    nj(nh nh2, ft ft2) {
        this.b = nh2;
        this.a = ft2;
    }

    public String a() {
        return this.a.getClass().getCanonicalName();
    }

    public /* synthetic */ Object call() {
        return this.a();
    }
}

