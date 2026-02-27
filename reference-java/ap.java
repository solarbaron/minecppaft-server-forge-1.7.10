/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import net.minecraft.server.MinecraftServer;

public class ap
extends y {
    @Override
    public String c() {
        return "xp";
    }

    @Override
    public int a() {
        return 2;
    }

    @Override
    public String c(ac ac2) {
        return "commands.xp.usage";
    }

    @Override
    public void b(ac ac2, String[] stringArray) {
        if (stringArray.length > 0) {
            int n2;
            boolean bl2;
            boolean bl3;
            String string = stringArray[0];
            boolean bl4 = bl3 = string.endsWith("l") || string.endsWith("L");
            if (bl3 && string.length() > 1) {
                string = string.substring(0, string.length() - 1);
            }
            boolean bl5 = bl2 = (n2 = ap.a(ac2, string)) < 0;
            if (bl2) {
                n2 *= -1;
            }
            mw mw2 = stringArray.length > 1 ? ap.d(ac2, stringArray[1]) : ap.b(ac2);
            if (bl3) {
                if (bl2) {
                    ((yz)mw2).a(-n2);
                    ap.a(ac2, (aa)this, "commands.xp.success.negative.levels", n2, mw2.b_());
                } else {
                    ((yz)mw2).a(n2);
                    ap.a(ac2, (aa)this, "commands.xp.success.levels", n2, mw2.b_());
                }
            } else {
                if (bl2) {
                    throw new ci("commands.xp.failure.widthdrawXp", new Object[0]);
                }
                mw2.v(n2);
                ap.a(ac2, (aa)this, "commands.xp.success", n2, mw2.b_());
            }
            return;
        }
        throw new ci("commands.xp.usage", new Object[0]);
    }

    @Override
    public List a(ac ac2, String[] stringArray) {
        if (stringArray.length == 2) {
            return ap.a(stringArray, this.d());
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

