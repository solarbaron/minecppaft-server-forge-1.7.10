/*
 * Decompiled with CFR 0.152.
 */
import java.util.concurrent.Callable;
import net.minecraft.server.MinecraftServer;

public class lk
implements Callable {
    final /* synthetic */ MinecraftServer a;

    public lk(MinecraftServer minecraftServer) {
        this.a = minecraftServer;
    }

    public String a() {
        return this.a.b.a ? this.a.b.c() : "N/A (disabled)";
    }

    public /* synthetic */ Object call() {
        return this.a();
    }
}

