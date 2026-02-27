/*
 * Decompiled with CFR 0.152.
 */
import com.mojang.authlib.GameProfile;
import java.util.List;
import net.minecraft.server.MinecraftServer;

public class ag
extends y {
    @Override
    public String c() {
        return "ban";
    }

    @Override
    public int a() {
        return 3;
    }

    @Override
    public String c(ac ac2) {
        return "commands.ban.usage";
    }

    @Override
    public boolean a(ac ac2) {
        return MinecraftServer.I().ah().h().b() && super.a(ac2);
    }

    @Override
    public void b(ac ac2, String[] stringArray) {
        if (stringArray.length >= 1 && stringArray[0].length() > 0) {
            MinecraftServer minecraftServer = MinecraftServer.I();
            GameProfile gameProfile = minecraftServer.ax().a(stringArray[0]);
            if (gameProfile == null) {
                throw new cd("commands.ban.failed", stringArray[0]);
            }
            String string = null;
            if (stringArray.length >= 2) {
                string = ag.a(ac2, stringArray, 1).c();
            }
            oq oq2 = new oq(gameProfile, null, ac2.b_(), null, string);
            minecraftServer.ah().h().a(oq2);
            mw mw2 = minecraftServer.ah().a(stringArray[0]);
            if (mw2 != null) {
                mw2.a.c("You are banned from this server.");
            }
            ag.a(ac2, (aa)this, "commands.ban.success", stringArray[0]);
            return;
        }
        throw new ci("commands.ban.usage", new Object[0]);
    }

    @Override
    public List a(ac ac2, String[] stringArray) {
        if (stringArray.length >= 1) {
            return ag.a(stringArray, MinecraftServer.I().E());
        }
        return null;
    }
}

