/*
 * Decompiled with CFR 0.152.
 */
public class uw
extends ui {
    private final wn a;
    private final double b;
    private int c;
    private int d;
    private int e;
    private int f;
    private int g;
    private int h;

    public uw(wn wn2, double d2) {
        this.a = wn2;
        this.b = d2;
        this.a(5);
    }

    @Override
    public boolean a() {
        return this.a.bZ() && !this.a.ca() && this.a.aI().nextDouble() <= (double)0.0065f && this.f();
    }

    @Override
    public boolean b() {
        return this.c <= this.e && this.d <= 60 && this.a(this.a.o, this.f, this.g, this.h);
    }

    @Override
    public void c() {
        this.a.m().a((double)this.f + 0.5, this.g + 1, (double)this.h + 0.5, this.b);
        this.c = 0;
        this.d = 0;
        this.e = this.a.aI().nextInt(this.a.aI().nextInt(1200) + 1200) + 1200;
        this.a.cc().a(false);
    }

    @Override
    public void d() {
        this.a.k(false);
    }

    @Override
    public void e() {
        ++this.c;
        this.a.cc().a(false);
        if (this.a.e(this.f, this.g + 1, this.h) > 1.0) {
            this.a.k(false);
            this.a.m().a((double)this.f + 0.5, this.g + 1, (double)this.h + 0.5, this.b);
            ++this.d;
        } else if (!this.a.ca()) {
            this.a.k(true);
        } else {
            --this.d;
        }
    }

    private boolean f() {
        int n2 = (int)this.a.t;
        double d2 = 2.147483647E9;
        int n3 = (int)this.a.s - 8;
        while ((double)n3 < this.a.s + 8.0) {
            int n4 = (int)this.a.u - 8;
            while ((double)n4 < this.a.u + 8.0) {
                double d3;
                if (this.a(this.a.o, n3, n2, n4) && this.a.o.c(n3, n2 + 1, n4) && (d3 = this.a.e(n3, n2, n4)) < d2) {
                    this.f = n3;
                    this.g = n2;
                    this.h = n4;
                    d2 = d3;
                }
                ++n4;
            }
            ++n3;
        }
        return d2 < 2.147483647E9;
    }

    private boolean a(ahb ahb2, int n2, int n3, int n4) {
        aji aji2 = ahb2.a(n2, n3, n4);
        int n5 = ahb2.e(n2, n3, n4);
        if (aji2 == ajn.ae) {
            aow aow2 = (aow)ahb2.o(n2, n3, n4);
            if (aow2.o < 1) {
                return true;
            }
        } else {
            if (aji2 == ajn.am) {
                return true;
            }
            if (aji2 == ajn.C && !ajh.b(n5)) {
                return true;
            }
        }
        return false;
    }
}

