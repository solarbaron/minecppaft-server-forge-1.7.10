/*
 * Decompiled with CFR 0.152.
 */
import net.minecraft.server.MinecraftServer;

public class mp
implements ahh {
    private MinecraftServer a;
    private mt b;

    public mp(MinecraftServer minecraftServer, mt mt2) {
        this.a = minecraftServer;
        this.b = mt2;
    }

    @Override
    public void a(String string, double d2, double d3, double d4, double d5, double d6, double d7) {
    }

    @Override
    public void a(sa sa2) {
        this.b.r().a(sa2);
    }

    @Override
    public void b(sa sa2) {
        this.b.r().b(sa2);
    }

    @Override
    public void a(String string, double d2, double d3, double d4, float f2, float f3) {
        this.a.ah().a(d2, d3, d4, f2 > 1.0f ? (double)(16.0f * f2) : 16.0, this.b.t.i, new hc(string, d2, d3, d4, f2, f3));
    }

    @Override
    public void a(yz yz2, String string, double d2, double d3, double d4, float f2, float f3) {
        this.a.ah().a(yz2, d2, d3, d4, f2 > 1.0f ? (double)(16.0f * f2) : 16.0, this.b.t.i, new hc(string, d2, d3, d4, f2, f3));
    }

    @Override
    public void a(int n2, int n3, int n4, int n5, int n6, int n7) {
    }

    @Override
    public void a(int n2, int n3, int n4) {
        this.b.t().a(n2, n3, n4);
    }

    @Override
    public void b(int n2, int n3, int n4) {
    }

    @Override
    public void a(String string, int n2, int n3, int n4) {
    }

    @Override
    public void a(yz yz2, int n2, int n3, int n4, int n5, int n6) {
        this.a.ah().a(yz2, n3, n4, n5, 64.0, this.b.t.i, new ha(n2, n3, n4, n5, n6, false));
    }

    @Override
    public void a(int n2, int n3, int n4, int n5, int n6) {
        this.a.ah().a(new ha(n2, n3, n4, n5, n6, true));
    }

    @Override
    public void b(int n2, int n3, int n4, int n5, int n6) {
        for (mw mw2 : this.a.ah().e) {
            double d2;
            double d3;
            double d4;
            if (mw2 == null || mw2.o != this.b || mw2.y() == n2 || !((d4 = (double)n3 - mw2.s) * d4 + (d3 = (double)n4 - mw2.t) * d3 + (d2 = (double)n5 - mw2.u) * d2 < 1024.0)) continue;
            mw2.a.a(new ge(n2, n3, n4, n5, n6));
        }
    }

    @Override
    public void b() {
    }
}

