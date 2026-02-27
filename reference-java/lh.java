/*
 * Decompiled with CFR 0.152.
 */
import net.minecraft.server.MinecraftServer;

public class lh
implements qk {
    private long b = MinecraftServer.ar();
    final /* synthetic */ MinecraftServer a;

    public lh(MinecraftServer minecraftServer) {
        this.a = minecraftServer;
    }

    @Override
    public void a(String string) {
    }

    @Override
    public void a(int n2) {
        if (MinecraftServer.ar() - this.b >= 1000L) {
            this.b = MinecraftServer.ar();
            MinecraftServer.aA().info("Converting... " + n2 + "%");
        }
    }

    @Override
    public void c(String string) {
    }
}

