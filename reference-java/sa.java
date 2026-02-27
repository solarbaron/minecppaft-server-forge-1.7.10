/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;
import java.util.UUID;
import net.minecraft.server.MinecraftServer;

public abstract class sa {
    private static int b;
    private int c = b++;
    public double j = 1.0;
    public boolean k;
    public sa l;
    public sa m;
    public boolean n;
    public ahb o;
    public double p;
    public double q;
    public double r;
    public double s;
    public double t;
    public double u;
    public double v;
    public double w;
    public double x;
    public float y;
    public float z;
    public float A;
    public float B;
    public final azt C = azt.a(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    public boolean D;
    public boolean E;
    public boolean F;
    public boolean G;
    public boolean H;
    protected boolean I;
    public boolean J = true;
    public boolean K;
    public float L;
    public float M = 0.6f;
    public float N = 1.8f;
    public float O;
    public float P;
    public float Q;
    public float R;
    private int d = 1;
    public double S;
    public double T;
    public double U;
    public float V;
    public float W;
    public boolean X;
    public float Y;
    protected Random Z = new Random();
    public int aa;
    public int ab = 1;
    private int e;
    protected boolean ac;
    public int ad;
    private boolean f = true;
    protected boolean ae;
    protected te af;
    private double g;
    private double h;
    public boolean ag;
    public int ah;
    public int ai;
    public int aj;
    public boolean ak;
    public boolean al;
    public int am;
    protected boolean an;
    protected int ao;
    public int ap;
    protected int aq;
    private boolean i;
    protected UUID ar = UUID.randomUUID();
    public se as = se.b;

    public int y() {
        return this.c;
    }

    public void d(int n2) {
        this.c = n2;
    }

    public sa(ahb ahb2) {
        this.o = ahb2;
        this.b(0.0, 0.0, 0.0);
        if (ahb2 != null) {
            this.ap = ahb2.t.i;
        }
        this.af = new te(this);
        this.af.a(0, (Object)0);
        this.af.a(1, (Object)300);
        this.c();
    }

    protected abstract void c();

    public te z() {
        return this.af;
    }

    public boolean equals(Object object) {
        if (object instanceof sa) {
            return ((sa)object).c == this.c;
        }
        return false;
    }

    public int hashCode() {
        return this.c;
    }

    public void B() {
        this.K = true;
    }

    protected void a(float f2, float f3) {
        float f4;
        if (f2 != this.M || f3 != this.N) {
            f4 = this.M;
            this.M = f2;
            this.N = f3;
            this.C.d = this.C.a + (double)this.M;
            this.C.f = this.C.c + (double)this.M;
            this.C.e = this.C.b + (double)this.N;
            if (this.M > f4 && !this.f && !this.o.E) {
                this.d(f4 - this.M, 0.0, f4 - this.M);
            }
        }
        this.as = (double)(f4 = f2 % 2.0f) < 0.375 ? se.a : ((double)f4 < 0.75 ? se.b : ((double)f4 < 1.0 ? se.c : ((double)f4 < 1.375 ? se.d : ((double)f4 < 1.75 ? se.e : se.f))));
    }

    protected void b(float f2, float f3) {
        this.y = f2 % 360.0f;
        this.z = f3 % 360.0f;
    }

    public void b(double d2, double d3, double d4) {
        this.s = d2;
        this.t = d3;
        this.u = d4;
        float f2 = this.M / 2.0f;
        float f3 = this.N;
        this.C.b(d2 - (double)f2, d3 - (double)this.L + (double)this.V, d4 - (double)f2, d2 + (double)f2, d3 - (double)this.L + (double)this.V + (double)f3, d4 + (double)f2);
    }

    public void h() {
        this.C();
    }

    public void C() {
        int n2;
        aji aji2;
        int n3;
        int n4;
        this.o.C.a("entityBaseTick");
        if (this.m != null && this.m.K) {
            this.m = null;
        }
        this.O = this.P;
        this.p = this.s;
        this.q = this.t;
        this.r = this.u;
        this.B = this.z;
        this.A = this.y;
        if (!this.o.E && this.o instanceof mt) {
            this.o.C.a("portal");
            MinecraftServer minecraftServer = ((mt)this.o).q();
            n4 = this.D();
            if (this.an) {
                if (minecraftServer.w()) {
                    if (this.m == null && this.ao++ >= n4) {
                        this.ao = n4;
                        this.am = this.ai();
                        n3 = this.o.t.i == -1 ? 0 : -1;
                        this.b(n3);
                    }
                    this.an = false;
                }
            } else {
                if (this.ao > 0) {
                    this.ao -= 4;
                }
                if (this.ao < 0) {
                    this.ao = 0;
                }
            }
            if (this.am > 0) {
                --this.am;
            }
            this.o.C.b();
        }
        if (this.ao() && !this.M() && (aji2 = this.o.a(n2 = qh.c(this.s), n4 = qh.c(this.t - (double)0.2f - (double)this.L), n3 = qh.c(this.u))).o() != awt.a) {
            this.o.a("blockcrack_" + aji.b(aji2) + "_" + this.o.e(n2, n4, n3), this.s + ((double)this.Z.nextFloat() - 0.5) * (double)this.M, this.C.b + 0.1, this.u + ((double)this.Z.nextFloat() - 0.5) * (double)this.M, -this.v * 4.0, 1.5, -this.x * 4.0);
        }
        this.N();
        if (this.o.E) {
            this.e = 0;
        } else if (this.e > 0) {
            if (this.ae) {
                this.e -= 4;
                if (this.e < 0) {
                    this.e = 0;
                }
            } else {
                if (this.e % 20 == 0) {
                    this.a(ro.b, 1.0f);
                }
                --this.e;
            }
        }
        if (this.P()) {
            this.E();
            this.R *= 0.5f;
        }
        if (this.t < -64.0) {
            this.G();
        }
        if (!this.o.E) {
            this.a(0, this.e > 0);
        }
        this.f = false;
        this.o.C.b();
    }

    public int D() {
        return 0;
    }

    protected void E() {
        if (!this.ae) {
            this.a(ro.c, 4.0f);
            this.e(15);
        }
    }

    public void e(int n2) {
        int n3 = n2 * 20;
        if (this.e < (n3 = agi.a(this, n3))) {
            this.e = n3;
        }
    }

    public void F() {
        this.e = 0;
    }

    protected void G() {
        this.B();
    }

    public boolean c(double d2, double d3, double d4) {
        azt azt2 = this.C.c(d2, d3, d4);
        List list = this.o.a(this, azt2);
        if (!list.isEmpty()) {
            return false;
        }
        return !this.o.d(azt2);
    }

    public void d(double d2, double d3, double d4) {
        int n2;
        double d5;
        double d6;
        int n3;
        int n4;
        boolean bl2;
        if (this.X) {
            this.C.d(d2, d3, d4);
            this.s = (this.C.a + this.C.d) / 2.0;
            this.t = this.C.b + (double)this.L - (double)this.V;
            this.u = (this.C.c + this.C.f) / 2.0;
            return;
        }
        this.o.C.a("move");
        this.V *= 0.4f;
        double d7 = this.s;
        double d8 = this.t;
        double d9 = this.u;
        if (this.I) {
            this.I = false;
            d2 *= 0.25;
            d3 *= (double)0.05f;
            d4 *= 0.25;
            this.v = 0.0;
            this.w = 0.0;
            this.x = 0.0;
        }
        double d10 = d2;
        double d11 = d3;
        double d12 = d4;
        azt azt2 = this.C.b();
        boolean bl3 = bl2 = this.D && this.an() && this instanceof yz;
        if (bl2) {
            double d13 = 0.05;
            while (d2 != 0.0 && this.o.a(this, this.C.c(d2, -1.0, 0.0)).isEmpty()) {
                d2 = d2 < d13 && d2 >= -d13 ? 0.0 : (d2 > 0.0 ? (d2 -= d13) : (d2 += d13));
                d10 = d2;
            }
            while (d4 != 0.0 && this.o.a(this, this.C.c(0.0, -1.0, d4)).isEmpty()) {
                d4 = d4 < d13 && d4 >= -d13 ? 0.0 : (d4 > 0.0 ? (d4 -= d13) : (d4 += d13));
                d12 = d4;
            }
            while (d2 != 0.0 && d4 != 0.0 && this.o.a(this, this.C.c(d2, -1.0, d4)).isEmpty()) {
                d2 = d2 < d13 && d2 >= -d13 ? 0.0 : (d2 > 0.0 ? (d2 -= d13) : (d2 += d13));
                d4 = d4 < d13 && d4 >= -d13 ? 0.0 : (d4 > 0.0 ? (d4 -= d13) : (d4 += d13));
                d10 = d2;
                d12 = d4;
            }
        }
        List list = this.o.a(this, this.C.a(d2, d3, d4));
        for (n4 = 0; n4 < list.size(); ++n4) {
            d3 = ((azt)list.get(n4)).b(this.C, d3);
        }
        this.C.d(0.0, d3, 0.0);
        if (!this.J && d11 != d3) {
            d4 = 0.0;
            d3 = 0.0;
            d2 = 0.0;
        }
        n4 = this.D || d11 != d3 && d11 < 0.0 ? 1 : 0;
        for (n3 = 0; n3 < list.size(); ++n3) {
            d2 = ((azt)list.get(n3)).a(this.C, d2);
        }
        this.C.d(d2, 0.0, 0.0);
        if (!this.J && d10 != d2) {
            d4 = 0.0;
            d3 = 0.0;
            d2 = 0.0;
        }
        for (n3 = 0; n3 < list.size(); ++n3) {
            d4 = ((azt)list.get(n3)).c(this.C, d4);
        }
        this.C.d(0.0, 0.0, d4);
        if (!this.J && d12 != d4) {
            d4 = 0.0;
            d3 = 0.0;
            d2 = 0.0;
        }
        if (this.W > 0.0f && n4 != 0 && (bl2 || this.V < 0.05f) && (d10 != d2 || d12 != d4)) {
            double d14 = d2;
            d6 = d3;
            d5 = d4;
            d2 = d10;
            d3 = this.W;
            d4 = d12;
            azt azt3 = this.C.b();
            this.C.d(azt2);
            list = this.o.a(this, this.C.a(d2, d3, d4));
            for (n2 = 0; n2 < list.size(); ++n2) {
                d3 = ((azt)list.get(n2)).b(this.C, d3);
            }
            this.C.d(0.0, d3, 0.0);
            if (!this.J && d11 != d3) {
                d4 = 0.0;
                d3 = 0.0;
                d2 = 0.0;
            }
            for (n2 = 0; n2 < list.size(); ++n2) {
                d2 = ((azt)list.get(n2)).a(this.C, d2);
            }
            this.C.d(d2, 0.0, 0.0);
            if (!this.J && d10 != d2) {
                d4 = 0.0;
                d3 = 0.0;
                d2 = 0.0;
            }
            for (n2 = 0; n2 < list.size(); ++n2) {
                d4 = ((azt)list.get(n2)).c(this.C, d4);
            }
            this.C.d(0.0, 0.0, d4);
            if (!this.J && d12 != d4) {
                d4 = 0.0;
                d3 = 0.0;
                d2 = 0.0;
            }
            if (!this.J && d11 != d3) {
                d4 = 0.0;
                d3 = 0.0;
                d2 = 0.0;
            } else {
                d3 = -this.W;
                for (n2 = 0; n2 < list.size(); ++n2) {
                    d3 = ((azt)list.get(n2)).b(this.C, d3);
                }
                this.C.d(0.0, d3, 0.0);
            }
            if (d14 * d14 + d5 * d5 >= d2 * d2 + d4 * d4) {
                d2 = d14;
                d3 = d6;
                d4 = d5;
                this.C.d(azt3);
            }
        }
        this.o.C.b();
        this.o.C.a("rest");
        this.s = (this.C.a + this.C.d) / 2.0;
        this.t = this.C.b + (double)this.L - (double)this.V;
        this.u = (this.C.c + this.C.f) / 2.0;
        this.E = d10 != d2 || d12 != d4;
        this.F = d11 != d3;
        this.D = d11 != d3 && d11 < 0.0;
        this.G = this.E || this.F;
        this.a(d3, this.D);
        if (d10 != d2) {
            this.v = 0.0;
        }
        if (d11 != d3) {
            this.w = 0.0;
        }
        if (d12 != d4) {
            this.x = 0.0;
        }
        double d15 = this.s - d7;
        d6 = this.t - d8;
        d5 = this.u - d9;
        if (this.g_() && !bl2 && this.m == null) {
            int n5 = qh.c(this.s);
            n2 = qh.c(this.t - (double)0.2f - (double)this.L);
            int n6 = qh.c(this.u);
            aji aji2 = this.o.a(n5, n2, n6);
            int n7 = this.o.a(n5, n2 - 1, n6).b();
            if (n7 == 11 || n7 == 32 || n7 == 21) {
                aji2 = this.o.a(n5, n2 - 1, n6);
            }
            if (aji2 != ajn.ap) {
                d6 = 0.0;
            }
            this.P = (float)((double)this.P + (double)qh.a(d15 * d15 + d5 * d5) * 0.6);
            this.Q = (float)((double)this.Q + (double)qh.a(d15 * d15 + d6 * d6 + d5 * d5) * 0.6);
            if (this.Q > (float)this.d && aji2.o() != awt.a) {
                this.d = (int)this.Q + 1;
                if (this.M()) {
                    float f2 = qh.a(this.v * this.v * (double)0.2f + this.w * this.w + this.x * this.x * (double)0.2f) * 0.35f;
                    if (f2 > 1.0f) {
                        f2 = 1.0f;
                    }
                    this.a(this.H(), f2, 1.0f + (this.Z.nextFloat() - this.Z.nextFloat()) * 0.4f);
                }
                this.a(n5, n2, n6, aji2);
                aji2.b(this.o, n5, n2, n6, this);
            }
        }
        try {
            this.I();
        }
        catch (Throwable throwable) {
            b b2 = b.a(throwable, "Checking entity block collision");
            k k2 = b2.a("Entity being checked for collision");
            this.a(k2);
            throw new s(b2);
        }
        boolean bl4 = this.L();
        if (this.o.e(this.C.e(0.001, 0.001, 0.001))) {
            this.f(1);
            if (!bl4) {
                ++this.e;
                if (this.e == 0) {
                    this.e(8);
                }
            }
        } else if (this.e <= 0) {
            this.e = -this.ab;
        }
        if (bl4 && this.e > 0) {
            this.a("random.fizz", 0.7f, 1.6f + (this.Z.nextFloat() - this.Z.nextFloat()) * 0.4f);
            this.e = -this.ab;
        }
        this.o.C.b();
    }

    protected String H() {
        return "game.neutral.swim";
    }

    protected void I() {
        int n2;
        int n3;
        int n4;
        int n5;
        int n6;
        int n7 = qh.c(this.C.a + 0.001);
        if (this.o.b(n7, n6 = qh.c(this.C.b + 0.001), n5 = qh.c(this.C.c + 0.001), n4 = qh.c(this.C.d - 0.001), n3 = qh.c(this.C.e - 0.001), n2 = qh.c(this.C.f - 0.001))) {
            for (int i2 = n7; i2 <= n4; ++i2) {
                for (int i3 = n6; i3 <= n3; ++i3) {
                    for (int i4 = n5; i4 <= n2; ++i4) {
                        aji aji2 = this.o.a(i2, i3, i4);
                        try {
                            aji2.a(this.o, i2, i3, i4, this);
                            continue;
                        }
                        catch (Throwable throwable) {
                            b b2 = b.a(throwable, "Colliding entity with block");
                            k k2 = b2.a("Block being collided with");
                            k.a(k2, i2, i3, i4, aji2, this.o.e(i2, i3, i4));
                            throw new s(b2);
                        }
                    }
                }
            }
        }
    }

    protected void a(int n2, int n3, int n4, aji aji2) {
        ajm ajm2 = aji2.H;
        if (this.o.a(n2, n3 + 1, n4) == ajn.aC) {
            ajm2 = ajn.aC.H;
            this.a(ajm2.e(), ajm2.c() * 0.15f, ajm2.d());
        } else if (!aji2.o().d()) {
            this.a(ajm2.e(), ajm2.c() * 0.15f, ajm2.d());
        }
    }

    public void a(String string, float f2, float f3) {
        this.o.a(this, string, f2, f3);
    }

    protected boolean g_() {
        return true;
    }

    protected void a(double d2, boolean bl2) {
        if (bl2) {
            if (this.R > 0.0f) {
                this.b(this.R);
                this.R = 0.0f;
            }
        } else if (d2 < 0.0) {
            this.R = (float)((double)this.R - d2);
        }
    }

    public azt J() {
        return null;
    }

    protected void f(int n2) {
        if (!this.ae) {
            this.a(ro.a, (float)n2);
        }
    }

    public final boolean K() {
        return this.ae;
    }

    protected void b(float f2) {
        if (this.l != null) {
            this.l.b(f2);
        }
    }

    public boolean L() {
        return this.ac || this.o.y(qh.c(this.s), qh.c(this.t), qh.c(this.u)) || this.o.y(qh.c(this.s), qh.c(this.t + (double)this.N), qh.c(this.u));
    }

    public boolean M() {
        return this.ac;
    }

    public boolean N() {
        if (this.o.a(this.C.b(0.0, -0.4f, 0.0).e(0.001, 0.001, 0.001), awt.h, this)) {
            if (!this.ac && !this.f) {
                float f2;
                float f3;
                float f4 = qh.a(this.v * this.v * (double)0.2f + this.w * this.w + this.x * this.x * (double)0.2f) * 0.2f;
                if (f4 > 1.0f) {
                    f4 = 1.0f;
                }
                this.a(this.O(), f4, 1.0f + (this.Z.nextFloat() - this.Z.nextFloat()) * 0.4f);
                float f5 = qh.c(this.C.b);
                int n2 = 0;
                while ((float)n2 < 1.0f + this.M * 20.0f) {
                    f3 = (this.Z.nextFloat() * 2.0f - 1.0f) * this.M;
                    f2 = (this.Z.nextFloat() * 2.0f - 1.0f) * this.M;
                    this.o.a("bubble", this.s + (double)f3, (double)(f5 + 1.0f), this.u + (double)f2, this.v, this.w - (double)(this.Z.nextFloat() * 0.2f), this.x);
                    ++n2;
                }
                n2 = 0;
                while ((float)n2 < 1.0f + this.M * 20.0f) {
                    f3 = (this.Z.nextFloat() * 2.0f - 1.0f) * this.M;
                    f2 = (this.Z.nextFloat() * 2.0f - 1.0f) * this.M;
                    this.o.a("splash", this.s + (double)f3, (double)(f5 + 1.0f), this.u + (double)f2, this.v, this.w, this.x);
                    ++n2;
                }
            }
            this.R = 0.0f;
            this.ac = true;
            this.e = 0;
        } else {
            this.ac = false;
        }
        return this.ac;
    }

    protected String O() {
        return "game.neutral.swim.splash";
    }

    public boolean a(awt awt2) {
        int n2;
        int n3;
        double d2 = this.t + (double)this.g();
        int n4 = qh.c(this.s);
        aji aji2 = this.o.a(n4, n3 = qh.d(qh.c(d2)), n2 = qh.c(this.u));
        if (aji2.o() == awt2) {
            float f2 = alw.b(this.o.e(n4, n3, n2)) - 0.11111111f;
            float f3 = (float)(n3 + 1) - f2;
            return d2 < (double)f3;
        }
        return false;
    }

    public float g() {
        return 0.0f;
    }

    public boolean P() {
        return this.o.a(this.C.b(-0.1f, -0.4f, -0.1f), awt.i);
    }

    public void a(float f2, float f3, float f4) {
        float f5 = f2 * f2 + f3 * f3;
        if (f5 < 1.0E-4f) {
            return;
        }
        if ((f5 = qh.c(f5)) < 1.0f) {
            f5 = 1.0f;
        }
        f5 = f4 / f5;
        float f6 = qh.a(this.y * (float)Math.PI / 180.0f);
        float f7 = qh.b(this.y * (float)Math.PI / 180.0f);
        this.v += (double)((f2 *= f5) * f7 - (f3 *= f5) * f6);
        this.x += (double)(f3 * f7 + f2 * f6);
    }

    public float d(float f2) {
        int n2;
        int n3 = qh.c(this.s);
        if (this.o.d(n3, 0, n2 = qh.c(this.u))) {
            double d2 = (this.C.e - this.C.b) * 0.66;
            int n4 = qh.c(this.t - (double)this.L + d2);
            return this.o.n(n3, n4, n2);
        }
        return 0.0f;
    }

    public void a(ahb ahb2) {
        this.o = ahb2;
    }

    public void a(double d2, double d3, double d4, float f2, float f3) {
        this.p = this.s = d2;
        this.q = this.t = d3;
        this.r = this.u = d4;
        this.A = this.y = f2;
        this.B = this.z = f3;
        this.V = 0.0f;
        double d5 = this.A - f2;
        if (d5 < -180.0) {
            this.A += 360.0f;
        }
        if (d5 >= 180.0) {
            this.A -= 360.0f;
        }
        this.b(this.s, this.t, this.u);
        this.b(f2, f3);
    }

    public void b(double d2, double d3, double d4, float f2, float f3) {
        this.p = this.s = d2;
        this.S = this.s;
        this.q = this.t = d3 + (double)this.L;
        this.T = this.t;
        this.r = this.u = d4;
        this.U = this.u;
        this.y = f2;
        this.z = f3;
        this.b(this.s, this.t, this.u);
    }

    public float e(sa sa2) {
        float f2 = (float)(this.s - sa2.s);
        float f3 = (float)(this.t - sa2.t);
        float f4 = (float)(this.u - sa2.u);
        return qh.c(f2 * f2 + f3 * f3 + f4 * f4);
    }

    public double e(double d2, double d3, double d4) {
        double d5 = this.s - d2;
        double d6 = this.t - d3;
        double d7 = this.u - d4;
        return d5 * d5 + d6 * d6 + d7 * d7;
    }

    public double f(double d2, double d3, double d4) {
        double d5 = this.s - d2;
        double d6 = this.t - d3;
        double d7 = this.u - d4;
        return qh.a(d5 * d5 + d6 * d6 + d7 * d7);
    }

    public double f(sa sa2) {
        double d2 = this.s - sa2.s;
        double d3 = this.t - sa2.t;
        double d4 = this.u - sa2.u;
        return d2 * d2 + d3 * d3 + d4 * d4;
    }

    public void b_(yz yz2) {
    }

    public void g(sa sa2) {
        if (sa2.l == this || sa2.m == this) {
            return;
        }
        double d2 = sa2.s - this.s;
        double d3 = sa2.u - this.u;
        double d4 = qh.a(d2, d3);
        if (d4 >= (double)0.01f) {
            d4 = qh.a(d4);
            d2 /= d4;
            d3 /= d4;
            double d5 = 1.0 / d4;
            if (d5 > 1.0) {
                d5 = 1.0;
            }
            d2 *= d5;
            d3 *= d5;
            d2 *= (double)0.05f;
            d3 *= (double)0.05f;
            this.g(-(d2 *= (double)(1.0f - this.Y)), 0.0, -(d3 *= (double)(1.0f - this.Y)));
            sa2.g(d2, 0.0, d3);
        }
    }

    public void g(double d2, double d3, double d4) {
        this.v += d2;
        this.w += d3;
        this.x += d4;
        this.al = true;
    }

    protected void Q() {
        this.H = true;
    }

    public boolean a(ro ro2, float f2) {
        if (this.aw()) {
            return false;
        }
        this.Q();
        return false;
    }

    public boolean R() {
        return false;
    }

    public boolean S() {
        return false;
    }

    public void b(sa sa2, int n2) {
    }

    public boolean c(dh dh2) {
        String string = this.W();
        if (this.K || string == null) {
            return false;
        }
        dh2.a("id", string);
        this.e(dh2);
        return true;
    }

    public boolean d(dh dh2) {
        String string = this.W();
        if (this.K || string == null || this.l != null) {
            return false;
        }
        dh2.a("id", string);
        this.e(dh2);
        return true;
    }

    public void e(dh dh2) {
        try {
            dh dh3;
            dh2.a("Pos", this.a(this.s, this.t + (double)this.V, this.u));
            dh2.a("Motion", this.a(this.v, this.w, this.x));
            dh2.a("Rotation", this.a(new float[]{this.y, this.z}));
            dh2.a("FallDistance", this.R);
            dh2.a("Fire", (short)this.e);
            dh2.a("Air", (short)this.ar());
            dh2.a("OnGround", this.D);
            dh2.a("Dimension", this.ap);
            dh2.a("Invulnerable", this.i);
            dh2.a("PortalCooldown", this.am);
            dh2.a("UUIDMost", this.aB().getMostSignificantBits());
            dh2.a("UUIDLeast", this.aB().getLeastSignificantBits());
            this.b(dh2);
            if (this.m != null && this.m.c(dh3 = new dh())) {
                dh2.a("Riding", dh3);
            }
        }
        catch (Throwable throwable) {
            b b2 = b.a(throwable, "Saving entity NBT");
            k k2 = b2.a("Entity being saved");
            this.a(k2);
            throw new s(b2);
        }
    }

    public void f(dh dh2) {
        try {
            dq dq2 = dh2.c("Pos", 6);
            dq dq3 = dh2.c("Motion", 6);
            dq dq4 = dh2.c("Rotation", 5);
            this.v = dq3.d(0);
            this.w = dq3.d(1);
            this.x = dq3.d(2);
            if (Math.abs(this.v) > 10.0) {
                this.v = 0.0;
            }
            if (Math.abs(this.w) > 10.0) {
                this.w = 0.0;
            }
            if (Math.abs(this.x) > 10.0) {
                this.x = 0.0;
            }
            this.S = this.s = dq2.d(0);
            this.p = this.s;
            this.T = this.t = dq2.d(1);
            this.q = this.t;
            this.U = this.u = dq2.d(2);
            this.r = this.u;
            this.A = this.y = dq4.e(0);
            this.B = this.z = dq4.e(1);
            this.R = dh2.h("FallDistance");
            this.e = dh2.e("Fire");
            this.h(dh2.e("Air"));
            this.D = dh2.n("OnGround");
            this.ap = dh2.f("Dimension");
            this.i = dh2.n("Invulnerable");
            this.am = dh2.f("PortalCooldown");
            if (dh2.b("UUIDMost", 4) && dh2.b("UUIDLeast", 4)) {
                this.ar = new UUID(dh2.g("UUIDMost"), dh2.g("UUIDLeast"));
            }
            this.b(this.s, this.t, this.u);
            this.b(this.y, this.z);
            this.a(dh2);
            if (this.V()) {
                this.b(this.s, this.t, this.u);
            }
        }
        catch (Throwable throwable) {
            b b2 = b.a(throwable, "Loading entity NBT");
            k k2 = b2.a("Entity being loaded");
            this.a(k2);
            throw new s(b2);
        }
    }

    protected boolean V() {
        return true;
    }

    protected final String W() {
        return sg.b(this);
    }

    protected abstract void a(dh var1);

    protected abstract void b(dh var1);

    public void X() {
    }

    protected dq a(double ... dArray) {
        dq dq2 = new dq();
        for (double d2 : dArray) {
            dq2.a(new dk(d2));
        }
        return dq2;
    }

    protected dq a(float ... fArray) {
        dq dq2 = new dq();
        for (float f2 : fArray) {
            dq2.a(new dm(f2));
        }
        return dq2;
    }

    public xk a(adb adb2, int n2) {
        return this.a(adb2, n2, 0.0f);
    }

    public xk a(adb adb2, int n2, float f2) {
        return this.a(new add(adb2, n2, 0), f2);
    }

    public xk a(add add2, float f2) {
        if (add2.b == 0 || add2.b() == null) {
            return null;
        }
        xk xk2 = new xk(this.o, this.s, this.t + (double)f2, this.u, add2);
        xk2.b = 10;
        this.o.d(xk2);
        return xk2;
    }

    public boolean Z() {
        return !this.K;
    }

    public boolean aa() {
        for (int i2 = 0; i2 < 8; ++i2) {
            int n2;
            int n3;
            float f2 = ((float)((i2 >> 0) % 2) - 0.5f) * this.M * 0.8f;
            float f3 = ((float)((i2 >> 1) % 2) - 0.5f) * 0.1f;
            float f4 = ((float)((i2 >> 2) % 2) - 0.5f) * this.M * 0.8f;
            int n4 = qh.c(this.s + (double)f2);
            if (!this.o.a(n4, n3 = qh.c(this.t + (double)this.g() + (double)f3), n2 = qh.c(this.u + (double)f4)).r()) continue;
            return true;
        }
        return false;
    }

    public boolean c(yz yz2) {
        return false;
    }

    public azt h(sa sa2) {
        return null;
    }

    public void ab() {
        if (this.m.K) {
            this.m = null;
            return;
        }
        this.v = 0.0;
        this.w = 0.0;
        this.x = 0.0;
        this.h();
        if (this.m == null) {
            return;
        }
        this.m.ac();
        this.h += (double)(this.m.y - this.m.A);
        this.g += (double)(this.m.z - this.m.B);
        while (this.h >= 180.0) {
            this.h -= 360.0;
        }
        while (this.h < -180.0) {
            this.h += 360.0;
        }
        while (this.g >= 180.0) {
            this.g -= 360.0;
        }
        while (this.g < -180.0) {
            this.g += 360.0;
        }
        double d2 = this.h * 0.5;
        double d3 = this.g * 0.5;
        float f2 = 10.0f;
        if (d2 > (double)f2) {
            d2 = f2;
        }
        if (d2 < (double)(-f2)) {
            d2 = -f2;
        }
        if (d3 > (double)f2) {
            d3 = f2;
        }
        if (d3 < (double)(-f2)) {
            d3 = -f2;
        }
        this.h -= d2;
        this.g -= d3;
    }

    public void ac() {
        if (this.l == null) {
            return;
        }
        this.l.b(this.s, this.t + this.ae() + this.l.ad(), this.u);
    }

    public double ad() {
        return this.L;
    }

    public double ae() {
        return (double)this.N * 0.75;
    }

    public void a(sa sa2) {
        this.g = 0.0;
        this.h = 0.0;
        if (sa2 == null) {
            if (this.m != null) {
                this.b(this.m.s, this.m.C.b + (double)this.m.N, this.m.u, this.y, this.z);
                this.m.l = null;
            }
            this.m = null;
            return;
        }
        if (this.m != null) {
            this.m.l = null;
        }
        if (sa2 != null) {
            sa sa3 = sa2.m;
            while (sa3 != null) {
                if (sa3 == this) {
                    return;
                }
                sa3 = sa3.m;
            }
        }
        this.m = sa2;
        sa2.l = this;
    }

    public float af() {
        return 0.1f;
    }

    public azw ag() {
        return null;
    }

    public void ah() {
        if (this.am > 0) {
            this.am = this.ai();
            return;
        }
        double d2 = this.p - this.s;
        double d3 = this.r - this.u;
        if (!this.o.E && !this.an) {
            this.aq = p.a(d2, d3);
        }
        this.an = true;
    }

    public int ai() {
        return 300;
    }

    public add[] ak() {
        return null;
    }

    public void c(int n2, add add2) {
    }

    public boolean al() {
        boolean bl2 = this.o != null && this.o.E;
        return !this.ae && (this.e > 0 || bl2 && this.g(0));
    }

    public boolean am() {
        return this.m != null;
    }

    public boolean an() {
        return this.g(1);
    }

    public void b(boolean bl2) {
        this.a(1, bl2);
    }

    public boolean ao() {
        return this.g(3);
    }

    public void c(boolean bl2) {
        this.a(3, bl2);
    }

    public boolean ap() {
        return this.g(5);
    }

    public void d(boolean bl2) {
        this.a(5, bl2);
    }

    public void e(boolean bl2) {
        this.a(4, bl2);
    }

    protected boolean g(int n2) {
        return (this.af.a(0) & 1 << n2) != 0;
    }

    protected void a(int n2, boolean bl2) {
        byte by2 = this.af.a(0);
        if (bl2) {
            this.af.b(0, (byte)(by2 | 1 << n2));
        } else {
            this.af.b(0, (byte)(by2 & ~(1 << n2)));
        }
    }

    public int ar() {
        return this.af.b(1);
    }

    public void h(int n2) {
        this.af.b(1, (short)n2);
    }

    public void a(xh xh2) {
        this.f(5);
        ++this.e;
        if (this.e == 0) {
            this.e(8);
        }
    }

    public void a(sv sv2) {
    }

    protected boolean j(double d2, double d3, double d4) {
        int n2 = qh.c(d2);
        int n3 = qh.c(d3);
        int n4 = qh.c(d4);
        double d5 = d2 - (double)n2;
        double d6 = d3 - (double)n3;
        double d7 = d4 - (double)n4;
        List list = this.o.a(this.C);
        if (!list.isEmpty() || this.o.q(n2, n3, n4)) {
            boolean bl2 = !this.o.q(n2 - 1, n3, n4);
            boolean bl3 = !this.o.q(n2 + 1, n3, n4);
            boolean bl4 = !this.o.q(n2, n3 - 1, n4);
            boolean bl5 = !this.o.q(n2, n3 + 1, n4);
            boolean bl6 = !this.o.q(n2, n3, n4 - 1);
            boolean bl7 = !this.o.q(n2, n3, n4 + 1);
            int n5 = 3;
            double d8 = 9999.0;
            if (bl2 && d5 < d8) {
                d8 = d5;
                n5 = 0;
            }
            if (bl3 && 1.0 - d5 < d8) {
                d8 = 1.0 - d5;
                n5 = 1;
            }
            if (bl5 && 1.0 - d6 < d8) {
                d8 = 1.0 - d6;
                n5 = 3;
            }
            if (bl6 && d7 < d8) {
                d8 = d7;
                n5 = 4;
            }
            if (bl7 && 1.0 - d7 < d8) {
                d8 = 1.0 - d7;
                n5 = 5;
            }
            float f2 = this.Z.nextFloat() * 0.2f + 0.1f;
            if (n5 == 0) {
                this.v = -f2;
            }
            if (n5 == 1) {
                this.v = f2;
            }
            if (n5 == 2) {
                this.w = -f2;
            }
            if (n5 == 3) {
                this.w = f2;
            }
            if (n5 == 4) {
                this.x = -f2;
            }
            if (n5 == 5) {
                this.x = f2;
            }
            return true;
        }
        return false;
    }

    public void as() {
        this.I = true;
        this.R = 0.0f;
    }

    public String b_() {
        String string = sg.b(this);
        if (string == null) {
            string = "generic";
        }
        return dd.a("entity." + string + ".name");
    }

    public sa[] at() {
        return null;
    }

    public boolean i(sa sa2) {
        return this == sa2;
    }

    public float au() {
        return 0.0f;
    }

    public boolean av() {
        return true;
    }

    public boolean j(sa sa2) {
        return false;
    }

    public String toString() {
        return String.format("%s['%s'/%d, l='%s', x=%.2f, y=%.2f, z=%.2f]", this.getClass().getSimpleName(), this.b_(), this.c, this.o == null ? "~NULL~" : this.o.N().k(), this.s, this.t, this.u);
    }

    public boolean aw() {
        return this.i;
    }

    public void k(sa sa2) {
        this.b(sa2.s, sa2.t, sa2.u, sa2.y, sa2.z);
    }

    public void a(sa sa2, boolean bl2) {
        dh dh2 = new dh();
        sa2.e(dh2);
        this.f(dh2);
        this.am = sa2.am;
        this.aq = sa2.aq;
    }

    public void b(int n2) {
        if (this.o.E || this.K) {
            return;
        }
        this.o.C.a("changeDimension");
        MinecraftServer minecraftServer = MinecraftServer.I();
        int n3 = this.ap;
        mt mt2 = minecraftServer.a(n3);
        mt mt3 = minecraftServer.a(n2);
        this.ap = n2;
        if (n3 == 1 && n2 == 1) {
            mt3 = minecraftServer.a(0);
            this.ap = 0;
        }
        this.o.e(this);
        this.K = false;
        this.o.C.a("reposition");
        minecraftServer.ah().a(this, n3, mt2, mt3);
        this.o.C.c("reloading");
        sa sa2 = sg.a(sg.b(this), (ahb)mt3);
        if (sa2 != null) {
            sa2.a(this, true);
            if (n3 == 1 && n2 == 1) {
                r r2 = mt3.K();
                r2.b = this.o.i(r2.a, r2.c);
                sa2.b(r2.a, r2.b, r2.c, sa2.y, sa2.z);
            }
            mt3.d(sa2);
        }
        this.K = true;
        this.o.C.b();
        mt2.i();
        mt3.i();
        this.o.C.b();
    }

    public float a(agw agw2, ahb ahb2, int n2, int n3, int n4, aji aji2) {
        return aji2.a(this);
    }

    public boolean a(agw agw2, ahb ahb2, int n2, int n3, int n4, aji aji2, float f2) {
        return true;
    }

    public int ax() {
        return 3;
    }

    public int ay() {
        return this.aq;
    }

    public boolean az() {
        return false;
    }

    public void a(k k2) {
        k2.a("Entity Type", new sb(this));
        k2.a("Entity ID", this.c);
        k2.a("Entity Name", new sc(this));
        k2.a("Entity's Exact location", String.format("%.2f, %.2f, %.2f", this.s, this.t, this.u));
        k2.a("Entity's Block location", k.a(qh.c(this.s), qh.c(this.t), qh.c(this.u)));
        k2.a("Entity's Momentum", String.format("%.2f, %.2f, %.2f", this.v, this.w, this.x));
    }

    public UUID aB() {
        return this.ar;
    }

    public boolean aC() {
        return true;
    }

    public fj c_() {
        return new fq(this.b_());
    }

    public void i(int n2) {
    }
}

