/*
 * Decompiled with CFR 0.152.
 */
import com.mojang.authlib.GameProfile;
import java.util.List;
import net.minecraft.server.MinecraftServer;

public class aj
extends y {
    @Override
    public String c() {
        return "deop";
    }

    @Override
    public int a() {
        return 3;
    }

    @Override
    public String c(ac ac2) {
        return "commands.deop.usage";
    }

    @Override
    public void b(ac ac2, String[] stringArray) {
        if (stringArray.length == 1 && stringArray[0].length() > 0) {
            MinecraftServer minecraftServer = MinecraftServer.I();
            GameProfile gameProfile = minecraftServer.ah().m().a(stringArray[0]);
            if (gameProfile == null) {
                throw new cd("commands.deop.failed", stringArray[0]);
            }
            minecraftServer.ah().b(gameProfile);
            aj.a(ac2, (aa)this, "commands.deop.success", stringArray[0]);
            return;
        }
        throw new ci("commands.deop.usage", new Object[0]);
    }

    @Override
    public List a(ac ac2, String[] stringArray) {
        if (stringArray.length == 1) {
            return aj.a(stringArray, MinecraftServer.I().ah().n());
        }
        return null;
    }
}

