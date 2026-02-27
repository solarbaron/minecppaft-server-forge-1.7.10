/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import net.minecraft.server.MinecraftServer;

public class ah
extends y {
    @Override
    public String c() {
        return "say";
    }

    @Override
    public int a() {
        return 1;
    }

    @Override
    public String c(ac ac2) {
        return "commands.say.usage";
    }

    @Override
    public void b(ac ac2, String[] stringArray) {
        if (stringArray.length > 0 && stringArray[0].length() > 0) {
            fj fj2 = ah.a(ac2, stringArray, 0, true);
            MinecraftServer.I().ah().a(new fr("chat.type.announcement", ac2.b_(), fj2));
            return;
        }
        throw new ci("commands.say.usage", new Object[0]);
    }

    @Override
    public List a(ac ac2, String[] stringArray) {
        if (stringArray.length >= 1) {
            return ah.a(stringArray, MinecraftServer.I().E());
        }
        return null;
    }
}

