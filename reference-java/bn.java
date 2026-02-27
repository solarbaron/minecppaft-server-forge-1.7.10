/*
 * Decompiled with CFR 0.152.
 */
public class bn
extends y {
    @Override
    public String c() {
        return "setworldspawn";
    }

    @Override
    public int a() {
        return 2;
    }

    @Override
    public String c(ac ac2) {
        return "commands.setworldspawn.usage";
    }

    /*
     * Enabled force condition propagation
     * Lifted jumps to return sites
     */
    @Override
    public void b(ac ac2, String[] stringArray) {
        if (stringArray.length == 3) {
            if (ac2.d() == null) throw new ci("commands.setworldspawn.usage", new Object[0]);
            int n2 = 0;
            int n3 = bn.a(ac2, stringArray[n2++], -30000000, 30000000);
            int n4 = bn.a(ac2, stringArray[n2++], 0, 256);
            int n5 = bn.a(ac2, stringArray[n2++], -30000000, 30000000);
            ac2.d().x(n3, n4, n5);
            bn.a(ac2, (aa)this, "commands.setworldspawn.success", n3, n4, n5);
            return;
        } else {
            if (stringArray.length != 0) throw new ci("commands.setworldspawn.usage", new Object[0]);
            r r2 = bn.b(ac2).f_();
            ac2.d().x(r2.a, r2.b, r2.c);
            bn.a(ac2, (aa)this, "commands.setworldspawn.success", r2.a, r2.b, r2.c);
        }
    }
}

