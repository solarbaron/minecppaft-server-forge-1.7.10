/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import net.minecraft.server.MinecraftServer;

public class ax
extends y {
    @Override
    public String c() {
        return "banlist";
    }

    @Override
    public int a() {
        return 3;
    }

    @Override
    public boolean a(ac ac2) {
        return (MinecraftServer.I().ah().i().b() || MinecraftServer.I().ah().h().b()) && super.a(ac2);
    }

    @Override
    public String c(ac ac2) {
        return "commands.banlist.usage";
    }

    @Override
    public void b(ac ac2, String[] stringArray) {
        if (stringArray.length >= 1 && stringArray[0].equalsIgnoreCase("ips")) {
            ac2.a(new fr("commands.banlist.ips", MinecraftServer.I().ah().i().a().length));
            ac2.a(new fq(ax.a(MinecraftServer.I().ah().i().a())));
        } else {
            ac2.a(new fr("commands.banlist.players", MinecraftServer.I().ah().h().a().length));
            ac2.a(new fq(ax.a(MinecraftServer.I().ah().h().a())));
        }
    }

    @Override
    public List a(ac ac2, String[] stringArray) {
        if (stringArray.length == 1) {
            return ax.a(stringArray, "players", "ips");
        }
        return null;
    }
}

