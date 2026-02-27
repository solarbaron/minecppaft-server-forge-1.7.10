/*
 * Decompiled with CFR 0.152.
 */
import com.mojang.authlib.GameProfile;
import java.util.List;
import net.minecraft.server.MinecraftServer;

public class cc
extends y {
    @Override
    public String c() {
        return "whitelist";
    }

    @Override
    public int a() {
        return 3;
    }

    @Override
    public String c(ac ac2) {
        return "commands.whitelist.usage";
    }

    @Override
    public void b(ac ac2, String[] stringArray) {
        if (stringArray.length >= 1) {
            MinecraftServer minecraftServer = MinecraftServer.I();
            if (stringArray[0].equals("on")) {
                minecraftServer.ah().a(true);
                cc.a(ac2, (aa)this, "commands.whitelist.enabled", new Object[0]);
                return;
            }
            if (stringArray[0].equals("off")) {
                minecraftServer.ah().a(false);
                cc.a(ac2, (aa)this, "commands.whitelist.disabled", new Object[0]);
                return;
            }
            if (stringArray[0].equals("list")) {
                ac2.a(new fr("commands.whitelist.list", minecraftServer.ah().l().length, minecraftServer.ah().q().length));
                Object[] objectArray = minecraftServer.ah().l();
                ac2.a(new fq(cc.a(objectArray)));
                return;
            }
            if (stringArray[0].equals("add")) {
                if (stringArray.length < 2) {
                    throw new ci("commands.whitelist.add.usage", new Object[0]);
                }
                GameProfile gameProfile = minecraftServer.ax().a(stringArray[1]);
                if (gameProfile == null) {
                    throw new cd("commands.whitelist.add.failed", stringArray[1]);
                }
                minecraftServer.ah().d(gameProfile);
                cc.a(ac2, (aa)this, "commands.whitelist.add.success", stringArray[1]);
                return;
            }
            if (stringArray[0].equals("remove")) {
                if (stringArray.length < 2) {
                    throw new ci("commands.whitelist.remove.usage", new Object[0]);
                }
                GameProfile gameProfile = minecraftServer.ah().k().a(stringArray[1]);
                if (gameProfile == null) {
                    throw new cd("commands.whitelist.remove.failed", stringArray[1]);
                }
                minecraftServer.ah().c(gameProfile);
                cc.a(ac2, (aa)this, "commands.whitelist.remove.success", stringArray[1]);
                return;
            }
            if (stringArray[0].equals("reload")) {
                minecraftServer.ah().a();
                cc.a(ac2, (aa)this, "commands.whitelist.reloaded", new Object[0]);
                return;
            }
        }
        throw new ci("commands.whitelist.usage", new Object[0]);
    }

    @Override
    public List a(ac ac2, String[] stringArray) {
        if (stringArray.length == 1) {
            return cc.a(stringArray, "on", "off", "list", "add", "remove", "reload");
        }
        if (stringArray.length == 2) {
            if (stringArray[0].equals("remove")) {
                return cc.a(stringArray, MinecraftServer.I().ah().l());
            }
            if (stringArray[0].equals("add")) {
                return cc.a(stringArray, MinecraftServer.I().ax().a());
            }
        }
        return null;
    }
}

