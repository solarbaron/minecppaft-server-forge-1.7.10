/*
 * Decompiled with CFR 0.152.
 */
import net.minecraft.server.MinecraftServer;

public class bo
extends y {
    @Override
    public String c() {
        return "setidletimeout";
    }

    @Override
    public int a() {
        return 3;
    }

    @Override
    public String c(ac ac2) {
        return "commands.setidletimeout.usage";
    }

    @Override
    public void b(ac ac2, String[] stringArray) {
        if (stringArray.length == 1) {
            int n2 = bo.a(ac2, stringArray[0], 0);
            MinecraftServer.I().d(n2);
            bo.a(ac2, (aa)this, "commands.setidletimeout.success", n2);
            return;
        }
        throw new ci("commands.setidletimeout.usage", new Object[0]);
    }
}

