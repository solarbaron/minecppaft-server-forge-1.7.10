/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import net.minecraft.server.MinecraftServer;

public class ao
extends y {
    @Override
    public String c() {
        return "enchant";
    }

    @Override
    public int a() {
        return 2;
    }

    @Override
    public String c(ac ac2) {
        return "commands.enchant.usage";
    }

    @Override
    public void b(ac ac2, String[] stringArray) {
        if (stringArray.length >= 2) {
            dq dq2;
            mw mw2 = ao.d(ac2, stringArray[0]);
            int n2 = ao.a(ac2, stringArray[1], 0, aft.b.length - 1);
            int n3 = 1;
            add add2 = mw2.bF();
            if (add2 == null) {
                throw new cd("commands.enchant.noItem", new Object[0]);
            }
            aft aft2 = aft.b[n2];
            if (aft2 == null) {
                throw new ce("commands.enchant.notFound", n2);
            }
            if (!aft2.a(add2)) {
                throw new cd("commands.enchant.cantEnchant", new Object[0]);
            }
            if (stringArray.length >= 3) {
                n3 = ao.a(ac2, stringArray[2], aft2.d(), aft2.b());
            }
            if (add2.p() && (dq2 = add2.r()) != null) {
                for (int i2 = 0; i2 < dq2.c(); ++i2) {
                    aft aft3;
                    short s2 = dq2.b(i2).e("id");
                    if (aft.b[s2] == null || (aft3 = aft.b[s2]).a(aft2)) continue;
                    throw new cd("commands.enchant.cantCombine", aft2.c(n3), aft3.c(dq2.b(i2).e("lvl")));
                }
            }
            add2.a(aft2, n3);
            ao.a(ac2, (aa)this, "commands.enchant.success", new Object[0]);
            return;
        }
        throw new ci("commands.enchant.usage", new Object[0]);
    }

    @Override
    public List a(ac ac2, String[] stringArray) {
        if (stringArray.length == 1) {
            return ao.a(stringArray, this.d());
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

