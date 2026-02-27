/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import net.minecraft.server.MinecraftServer;

public class af
extends y {
    public static final Pattern a = Pattern.compile("^([01]?\\d\\d?|2[0-4]\\d|25[0-5])\\.([01]?\\d\\d?|2[0-4]\\d|25[0-5])\\.([01]?\\d\\d?|2[0-4]\\d|25[0-5])\\.([01]?\\d\\d?|2[0-4]\\d|25[0-5])$");

    @Override
    public String c() {
        return "ban-ip";
    }

    @Override
    public int a() {
        return 3;
    }

    @Override
    public boolean a(ac ac2) {
        return MinecraftServer.I().ah().i().b() && super.a(ac2);
    }

    @Override
    public String c(ac ac2) {
        return "commands.banip.usage";
    }

    @Override
    public void b(ac ac2, String[] stringArray) {
        if (stringArray.length >= 1 && stringArray[0].length() > 1) {
            Matcher matcher = a.matcher(stringArray[0]);
            fj fj2 = null;
            if (stringArray.length >= 2) {
                fj2 = af.a(ac2, stringArray, 1);
            }
            if (matcher.matches()) {
                this.a(ac2, stringArray[0], fj2 == null ? null : fj2.c());
            } else {
                mw mw2 = MinecraftServer.I().ah().a(stringArray[0]);
                if (mw2 == null) {
                    throw new cg("commands.banip.invalid", new Object[0]);
                }
                this.a(ac2, mw2.s(), fj2 == null ? null : fj2.c());
            }
            return;
        }
        throw new ci("commands.banip.usage", new Object[0]);
    }

    @Override
    public List a(ac ac2, String[] stringArray) {
        if (stringArray.length == 1) {
            return af.a(stringArray, MinecraftServer.I().E());
        }
        return null;
    }

    protected void a(ac ac2, String string, String string2) {
        ny ny2 = new ny(string, null, ac2.b_(), null, string2);
        MinecraftServer.I().ah().i().a(ny2);
        List list = MinecraftServer.I().ah().b(string);
        Object[] objectArray = new String[list.size()];
        int n2 = 0;
        for (mw mw2 : list) {
            mw2.a.c("You have been IP banned.");
            objectArray[n2++] = mw2.b_();
        }
        if (list.isEmpty()) {
            af.a(ac2, (aa)this, "commands.banip.success", string);
        } else {
            af.a(ac2, (aa)this, "commands.banip.success.players", string, af.a(objectArray));
        }
    }
}

