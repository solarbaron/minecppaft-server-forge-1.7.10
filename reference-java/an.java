/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import net.minecraft.server.MinecraftServer;

public class an
extends y {
    @Override
    public String c() {
        return "me";
    }

    @Override
    public int a() {
        return 0;
    }

    @Override
    public String c(ac ac2) {
        return "commands.me.usage";
    }

    @Override
    public void b(ac ac2, String[] stringArray) {
        if (stringArray.length > 0) {
            fj fj2 = an.a(ac2, stringArray, 0, ac2.a(1, "me"));
            MinecraftServer.I().ah().a(new fr("chat.type.emote", ac2.c_(), fj2));
            return;
        }
        throw new ci("commands.me.usage", new Object[0]);
    }

    @Override
    public List a(ac ac2, String[] stringArray) {
        return an.a(stringArray, MinecraftServer.I().E());
    }
}

