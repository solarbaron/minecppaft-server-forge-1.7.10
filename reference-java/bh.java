/*
 * Decompiled with CFR 0.152.
 */
import net.minecraft.server.MinecraftServer;

public class bh
extends y {
    @Override
    public String c() {
        return "save-all";
    }

    @Override
    public String c(ac ac2) {
        return "commands.save.usage";
    }

    @Override
    public void b(ac ac2, String[] stringArray) {
        MinecraftServer minecraftServer = MinecraftServer.I();
        ac2.a(new fr("commands.save.start", new Object[0]));
        if (minecraftServer.ah() != null) {
            minecraftServer.ah().j();
        }
        try {
            boolean bl2;
            mt mt2;
            int n2;
            for (n2 = 0; n2 < minecraftServer.c.length; ++n2) {
                if (minecraftServer.c[n2] == null) continue;
                mt2 = minecraftServer.c[n2];
                bl2 = mt2.c;
                mt2.c = false;
                mt2.a(true, null);
                mt2.c = bl2;
            }
            if (stringArray.length > 0 && "flush".equals(stringArray[0])) {
                ac2.a(new fr("commands.save.flushStart", new Object[0]));
                for (n2 = 0; n2 < minecraftServer.c.length; ++n2) {
                    if (minecraftServer.c[n2] == null) continue;
                    mt2 = minecraftServer.c[n2];
                    bl2 = mt2.c;
                    mt2.c = false;
                    mt2.m();
                    mt2.c = bl2;
                }
                ac2.a(new fr("commands.save.flushEnd", new Object[0]));
            }
        }
        catch (ahg ahg2) {
            bh.a(ac2, (aa)this, "commands.save.failed", ahg2.getMessage());
            return;
        }
        bh.a(ac2, (aa)this, "commands.save.success", new Object[0]);
    }
}

