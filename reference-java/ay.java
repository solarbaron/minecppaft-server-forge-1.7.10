/*
 * Decompiled with CFR 0.152.
 */
import net.minecraft.server.MinecraftServer;

public class ay
extends y {
    @Override
    public String c() {
        return "list";
    }

    @Override
    public int a() {
        return 0;
    }

    @Override
    public String c(ac ac2) {
        return "commands.players.usage";
    }

    @Override
    public void b(ac ac2, String[] stringArray) {
        ac2.a(new fr("commands.players.list", MinecraftServer.I().C(), MinecraftServer.I().D()));
        ac2.a(new fq(MinecraftServer.I().ah().b(stringArray.length > 0 && "uuids".equalsIgnoreCase(stringArray[0]))));
    }
}

