/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import net.minecraft.server.MinecraftServer;

public class bv
extends y {
    @Override
    public String c() {
        return "tp";
    }

    @Override
    public int a() {
        return 2;
    }

    @Override
    public String c(ac ac2) {
        return "commands.tp.usage";
    }

    @Override
    public void b(ac ac2, String[] stringArray) {
        if (stringArray.length >= 1) {
            mw mw2;
            if (stringArray.length == 2 || stringArray.length == 4) {
                mw2 = bv.d(ac2, stringArray[0]);
                if (mw2 == null) {
                    throw new cg();
                }
            } else {
                mw2 = bv.b(ac2);
            }
            if (stringArray.length == 3 || stringArray.length == 4) {
                if (mw2.o != null) {
                    int n2 = stringArray.length - 3;
                    double d2 = bv.a(ac2, mw2.s, stringArray[n2++]);
                    double d3 = bv.a(ac2, mw2.t, stringArray[n2++], 0, 0);
                    double d4 = bv.a(ac2, mw2.u, stringArray[n2++]);
                    mw2.a((sa)null);
                    mw2.a(d2, d3, d4);
                    bv.a(ac2, (aa)this, "commands.tp.success.coordinates", mw2.b_(), d2, d3, d4);
                }
            } else if (stringArray.length == 1 || stringArray.length == 2) {
                mw mw3 = bv.d(ac2, stringArray[stringArray.length - 1]);
                if (mw3 == null) {
                    throw new cg();
                }
                if (mw3.o != mw2.o) {
                    bv.a(ac2, (aa)this, "commands.tp.notSameDimension", new Object[0]);
                    return;
                }
                mw2.a((sa)null);
                mw2.a.a(mw3.s, mw3.t, mw3.u, mw3.y, mw3.z);
                bv.a(ac2, (aa)this, "commands.tp.success", mw2.b_(), mw3.b_());
            }
            return;
        }
        throw new ci("commands.tp.usage", new Object[0]);
    }

    @Override
    public List a(ac ac2, String[] stringArray) {
        if (stringArray.length == 1 || stringArray.length == 2) {
            return bv.a(stringArray, MinecraftServer.I().E());
        }
        return null;
    }

    @Override
    public boolean a(String[] stringArray, int n2) {
        return n2 == 0;
    }
}

