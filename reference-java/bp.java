/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import net.minecraft.server.MinecraftServer;

public class bp
extends y {
    @Override
    public String c() {
        return "spawnpoint";
    }

    @Override
    public int a() {
        return 2;
    }

    @Override
    public String c(ac ac2) {
        return "commands.spawnpoint.usage";
    }

    @Override
    public void b(ac ac2, String[] stringArray) {
        mw mw2;
        mw mw3 = mw2 = stringArray.length == 0 ? bp.b(ac2) : bp.d(ac2, stringArray[0]);
        if (stringArray.length == 4) {
            if (mw2.o != null) {
                int n2 = 1;
                int n3 = 30000000;
                int n4 = bp.a(ac2, stringArray[n2++], -n3, n3);
                int n5 = bp.a(ac2, stringArray[n2++], 0, 256);
                int n6 = bp.a(ac2, stringArray[n2++], -n3, n3);
                mw2.a(new r(n4, n5, n6), true);
                bp.a(ac2, (aa)this, "commands.spawnpoint.success", mw2.b_(), n4, n5, n6);
            }
        } else if (stringArray.length <= 1) {
            r r2 = mw2.f_();
            mw2.a(r2, true);
            bp.a(ac2, (aa)this, "commands.spawnpoint.success", mw2.b_(), r2.a, r2.b, r2.c);
        } else {
            throw new ci("commands.spawnpoint.usage", new Object[0]);
        }
    }

    @Override
    public List a(ac ac2, String[] stringArray) {
        if (stringArray.length == 1 || stringArray.length == 2) {
            return bp.a(stringArray, MinecraftServer.I().E());
        }
        return null;
    }

    @Override
    public boolean a(String[] stringArray, int n2) {
        return n2 == 0;
    }
}

