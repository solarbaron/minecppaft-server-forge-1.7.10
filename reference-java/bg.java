/*
 * Decompiled with CFR 0.152.
 */
import net.minecraft.server.MinecraftServer;

public class bg
extends y {
    @Override
    public String c() {
        return "publish";
    }

    @Override
    public String c(ac ac2) {
        return "commands.publish.usage";
    }

    @Override
    public void b(ac ac2, String[] stringArray) {
        String string = MinecraftServer.I().a(ahk.b, false);
        if (string != null) {
            bg.a(ac2, (aa)this, "commands.publish.started", string);
        } else {
            bg.a(ac2, (aa)this, "commands.publish.failed", new Object[0]);
        }
    }
}

