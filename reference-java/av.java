/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import net.minecraft.server.MinecraftServer;

public class av
extends y {
    @Override
    public String c() {
        return "kick";
    }

    @Override
    public int a() {
        return 2;
    }

    @Override
    public String c(ac ac2) {
        return "commands.kick.usage";
    }

    @Override
    public void b(ac ac2, String[] stringArray) {
        if (stringArray.length > 0 && stringArray[0].length() > 1) {
            mw mw2 = MinecraftServer.I().ah().a(stringArray[0]);
            String string = "Kicked by an operator.";
            boolean bl2 = false;
            if (mw2 == null) {
                throw new cg();
            }
            if (stringArray.length >= 2) {
                string = av.a(ac2, stringArray, 1).c();
                bl2 = true;
            }
            mw2.a.c(string);
            if (bl2) {
                av.a(ac2, (aa)this, "commands.kick.success.reason", mw2.b_(), string);
            } else {
                av.a(ac2, (aa)this, "commands.kick.success", mw2.b_());
            }
            return;
        }
        throw new ci("commands.kick.usage", new Object[0]);
    }

    @Override
    public List a(ac ac2, String[] stringArray) {
        if (stringArray.length >= 1) {
            return av.a(stringArray, MinecraftServer.I().E());
        }
        return null;
    }
}

