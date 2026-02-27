/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import net.minecraft.server.MinecraftServer;

public class ar
extends y {
    @Override
    public String c() {
        return "gamemode";
    }

    @Override
    public int a() {
        return 2;
    }

    @Override
    public String c(ac ac2) {
        return "commands.gamemode.usage";
    }

    @Override
    public void b(ac ac2, String[] stringArray) {
        if (stringArray.length > 0) {
            ahk ahk2 = this.h(ac2, stringArray[0]);
            mw mw2 = stringArray.length >= 2 ? ar.d(ac2, stringArray[1]) : ar.b(ac2);
            ((yz)mw2).a(ahk2);
            mw2.R = 0.0f;
            fr fr2 = new fr("gameMode." + ahk2.b(), new Object[0]);
            if (mw2 != ac2) {
                ar.a(ac2, (aa)this, 1, "commands.gamemode.success.other", mw2.b_(), fr2);
            } else {
                ar.a(ac2, (aa)this, 1, "commands.gamemode.success.self", fr2);
            }
            return;
        }
        throw new ci("commands.gamemode.usage", new Object[0]);
    }

    protected ahk h(ac ac2, String string) {
        if (string.equalsIgnoreCase(ahk.b.b()) || string.equalsIgnoreCase("s")) {
            return ahk.b;
        }
        if (string.equalsIgnoreCase(ahk.c.b()) || string.equalsIgnoreCase("c")) {
            return ahk.c;
        }
        if (string.equalsIgnoreCase(ahk.d.b()) || string.equalsIgnoreCase("a")) {
            return ahk.d;
        }
        return ahj.a(ar.a(ac2, string, 0, ahk.values().length - 2));
    }

    @Override
    public List a(ac ac2, String[] stringArray) {
        if (stringArray.length == 1) {
            return ar.a(stringArray, "survival", "creative", "adventure");
        }
        if (stringArray.length == 2) {
            return ar.a(stringArray, this.d());
        }
        return null;
    }

    protected String[] d() {
        return MinecraftServer.I().E();
    }

    @Override
    public boolean a(String[] stringArray, int n2) {
        return n2 == 1;
    }
}

