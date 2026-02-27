/*
 * Decompiled with CFR 0.152.
 */
import net.minecraft.server.MinecraftServer;

public class ca
extends y {
    @Override
    public String c() {
        return "toggledownfall";
    }

    @Override
    public int a() {
        return 2;
    }

    @Override
    public String c(ac ac2) {
        return "commands.downfall.usage";
    }

    @Override
    public void b(ac ac2, String[] stringArray) {
        this.d();
        ca.a(ac2, (aa)this, "commands.downfall.success", new Object[0]);
    }

    protected void d() {
        ays ays2;
        ays2.b(!(ays2 = MinecraftServer.I().c[0].N()).p());
    }
}

