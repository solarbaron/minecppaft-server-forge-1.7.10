/*
 * Decompiled with CFR 0.152.
 */
import com.mojang.authlib.GameProfile;
import java.util.ArrayList;
import java.util.List;
import net.minecraft.server.MinecraftServer;

public class bc
extends y {
    @Override
    public String c() {
        return "op";
    }

    @Override
    public int a() {
        return 3;
    }

    @Override
    public String c(ac ac2) {
        return "commands.op.usage";
    }

    @Override
    public void b(ac ac2, String[] stringArray) {
        if (stringArray.length == 1 && stringArray[0].length() > 0) {
            MinecraftServer minecraftServer = MinecraftServer.I();
            GameProfile gameProfile = minecraftServer.ax().a(stringArray[0]);
            if (gameProfile == null) {
                throw new cd("commands.op.failed", stringArray[0]);
            }
            minecraftServer.ah().a(gameProfile);
            bc.a(ac2, (aa)this, "commands.op.success", stringArray[0]);
            return;
        }
        throw new ci("commands.op.usage", new Object[0]);
    }

    @Override
    public List a(ac ac2, String[] stringArray) {
        if (stringArray.length == 1) {
            String string = stringArray[stringArray.length - 1];
            ArrayList<String> arrayList = new ArrayList<String>();
            for (GameProfile gameProfile : MinecraftServer.I().F()) {
                if (MinecraftServer.I().ah().g(gameProfile) || !bc.a(string, gameProfile.getName())) continue;
                arrayList.add(gameProfile.getName());
            }
            return arrayList;
        }
        return null;
    }
}

