/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import net.minecraft.server.MinecraftServer;

public class at
extends y {
    @Override
    public String c() {
        return "give";
    }

    @Override
    public int a() {
        return 2;
    }

    @Override
    public String c(ac ac2) {
        return "commands.give.usage";
    }

    @Override
    public void b(ac ac2, String[] stringArray) {
        Object object;
        if (stringArray.length < 2) {
            throw new ci("commands.give.usage", new Object[0]);
        }
        mw mw2 = at.d(ac2, stringArray[0]);
        adb adb2 = at.f(ac2, stringArray[1]);
        int n2 = 1;
        int n3 = 0;
        if (stringArray.length >= 3) {
            n2 = at.a(ac2, stringArray[2], 1, 64);
        }
        if (stringArray.length >= 4) {
            n3 = at.a(ac2, stringArray[3]);
        }
        add add2 = new add(adb2, n2, n3);
        if (stringArray.length >= 5) {
            object = at.a(ac2, stringArray, 4).c();
            try {
                dy dy2 = eb.a((String)object);
                if (!(dy2 instanceof dh)) {
                    at.a(ac2, (aa)this, "commands.give.tagError", "Not a valid tag");
                    return;
                }
                add2.d((dh)dy2);
            }
            catch (ea ea2) {
                at.a(ac2, (aa)this, "commands.give.tagError", ea2.getMessage());
                return;
            }
        }
        object = mw2.a(add2, false);
        ((xk)object).b = 0;
        ((xk)object).a(mw2.b_());
        at.a(ac2, (aa)this, "commands.give.success", add2.E(), n2, mw2.b_());
    }

    @Override
    public List a(ac ac2, String[] stringArray) {
        if (stringArray.length == 1) {
            return at.a(stringArray, this.d());
        }
        if (stringArray.length == 2) {
            return at.a(stringArray, adb.e.b());
        }
        return null;
    }

    protected String[] d() {
        return MinecraftServer.I().E();
    }

    @Override
    public boolean a(String[] stringArray, int n2) {
        return n2 == 0;
    }
}

