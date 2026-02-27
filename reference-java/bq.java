/*
 * Decompiled with CFR 0.152.
 */
import net.minecraft.server.MinecraftServer;

public class bq
extends y {
    @Override
    public boolean a(ac ac2) {
        return MinecraftServer.I().N() || super.a(ac2);
    }

    @Override
    public String c() {
        return "seed";
    }

    @Override
    public int a() {
        return 2;
    }

    @Override
    public String c(ac ac2) {
        return "commands.seed.usage";
    }

    @Override
    public void b(ac ac2, String[] stringArray) {
        ahb ahb2 = ac2 instanceof yz ? ((yz)ac2).o : MinecraftServer.I().a(0);
        ac2.a(new fr("commands.seed.success", ahb2.H()));
    }
}

