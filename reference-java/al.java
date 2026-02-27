/*
 * Decompiled with CFR 0.152.
 */
import net.minecraft.server.MinecraftServer;

public class al
extends ar {
    @Override
    public String c() {
        return "defaultgamemode";
    }

    @Override
    public String c(ac ac2) {
        return "commands.defaultgamemode.usage";
    }

    @Override
    public void b(ac ac2, String[] stringArray) {
        if (stringArray.length > 0) {
            ahk ahk2 = this.h(ac2, stringArray[0]);
            this.a(ahk2);
            al.a(ac2, (aa)this, "commands.defaultgamemode.success", new fr("gameMode." + ahk2.b(), new Object[0]));
            return;
        }
        throw new ci("commands.defaultgamemode.usage", new Object[0]);
    }

    protected void a(ahk ahk2) {
        MinecraftServer minecraftServer = MinecraftServer.I();
        minecraftServer.a(ahk2);
        if (minecraftServer.ap()) {
            for (mw mw2 : MinecraftServer.I().ah().e) {
                mw2.a(ahk2);
                mw2.R = 0.0f;
            }
        }
    }
}

