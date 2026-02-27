/*
 * Decompiled with CFR 0.152.
 */
import net.minecraft.server.MinecraftServer;

public class bj
extends y {
    @Override
    public String c() {
        return "save-on";
    }

    @Override
    public String c(ac ac2) {
        return "commands.save-on.usage";
    }

    @Override
    public void b(ac ac2, String[] stringArray) {
        MinecraftServer minecraftServer = MinecraftServer.I();
        boolean bl2 = false;
        for (int i2 = 0; i2 < minecraftServer.c.length; ++i2) {
            if (minecraftServer.c[i2] == null) continue;
            mt mt2 = minecraftServer.c[i2];
            if (!mt2.c) continue;
            mt2.c = false;
            bl2 = true;
        }
        if (!bl2) {
            throw new cd("commands.save-on.alreadyOn", new Object[0]);
        }
        bj.a(ac2, (aa)this, "commands.save.enabled", new Object[0]);
    }
}

