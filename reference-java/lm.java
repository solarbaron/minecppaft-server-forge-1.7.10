/*
 * Decompiled with CFR 0.152.
 */
import java.util.concurrent.Callable;
import net.minecraft.server.MinecraftServer;

public class lm
implements Callable {
    final /* synthetic */ MinecraftServer a;

    public lm(MinecraftServer minecraftServer) {
        this.a = minecraftServer;
    }

    public String a() {
        return MinecraftServer.a(this.a).o() + " / " + MinecraftServer.a(this.a).p() + "; " + MinecraftServer.a((MinecraftServer)this.a).e;
    }

    public /* synthetic */ Object call() {
        return this.a();
    }
}

