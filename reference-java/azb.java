/*
 * Decompiled with CFR 0.152.
 */
import java.util.concurrent.Callable;

class azb
implements Callable {
    final /* synthetic */ ays a;

    azb(ays ays2) {
        this.a = ays2;
    }

    public String a() {
        return String.format("Game mode: %s (ID %d). Hardcore: %b. Cheats: %b", ays.o(this.a).b(), ays.o(this.a).a(), ays.p(this.a), ays.q(this.a));
    }

    public /* synthetic */ Object call() {
        return this.a();
    }
}

