/*
 * Decompiled with CFR 0.152.
 */
import com.mojang.authlib.GameProfile;
import java.util.List;
import net.minecraft.server.MinecraftServer;

public class be
extends y {
    @Override
    public String c() {
        return "pardon";
    }

    @Override
    public int a() {
        return 3;
    }

    @Override
    public String c(ac ac2) {
        return "commands.unban.usage";
    }

    @Override
    public boolean a(ac ac2) {
        return MinecraftServer.I().ah().h().b() && super.a(ac2);
    }

    @Override
    public void b(ac ac2, String[] stringArray) {
        if (stringArray.length == 1 && stringArray[0].length() > 0) {
            MinecraftServer minecraftServer = MinecraftServer.I();
            GameProfile gameProfile = minecraftServer.ah().h().a(stringArray[0]);
            if (gameProfile == null) {
                throw new cd("commands.unban.failed", stringArray[0]);
            }
            minecraftServer.ah().h().c(gameProfile);
            be.a(ac2, (aa)this, "commands.unban.success", stringArray[0]);
            return;
        }
        throw new ci("commands.unban.usage", new Object[0]);
    }

    @Override
    public List a(ac ac2, String[] stringArray) {
        if (stringArray.length == 1) {
            return be.a(stringArray, MinecraftServer.I().ah().h().a());
        }
        return null;
    }
}

