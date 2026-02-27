/*
 * Decompiled with CFR 0.152.
 */
import net.minecraft.server.MinecraftServer;

public class bt
extends y {
    @Override
    public String c() {
        return "stop";
    }

    @Override
    public String c(ac ac2) {
        return "commands.stop.usage";
    }

    @Override
    public void b(ac ac2, String[] stringArray) {
        if (MinecraftServer.I().c != null) {
            bt.a(ac2, (aa)this, "commands.stop.start", new Object[0]);
        }
        MinecraftServer.I().r();
    }
}

