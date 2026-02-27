/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Random;

public class agw {
    public boolean a;
    public boolean b = true;
    private int i = 16;
    private Random j = new Random();
    private ahb k;
    public double c;
    public double d;
    public double e;
    public sa f;
    public float g;
    public List h = new ArrayList();
    private Map l = new HashMap();

    public agw(ahb ahb2, sa sa2, double d2, double d3, double d4, float f2) {
        this.k = ahb2;
        this.f = sa2;
        this.g = f2;
        this.c = d2;
        this.d = d3;
        this.e = d4;
    }

    public void a() {
        double d2;
        double d3;
        double d4;
        int n2;
        int n3;
        int n4;
        float f2 = this.g;
        HashSet<agt> hashSet = new HashSet<agt>();
        for (n4 = 0; n4 < this.i; ++n4) {
            for (n3 = 0; n3 < this.i; ++n3) {
                for (n2 = 0; n2 < this.i; ++n2) {
                    if (n4 != 0 && n4 != this.i - 1 && n3 != 0 && n3 != this.i - 1 && n2 != 0 && n2 != this.i - 1) continue;
                    double d5 = (float)n4 / ((float)this.i - 1.0f) * 2.0f - 1.0f;
                    double d6 = (float)n3 / ((float)this.i - 1.0f) * 2.0f - 1.0f;
                    double d7 = (float)n2 / ((float)this.i - 1.0f) * 2.0f - 1.0f;
                    double d8 = Math.sqrt(d5 * d5 + d6 * d6 + d7 * d7);
                    d5 /= d8;
                    d6 /= d8;
                    d7 /= d8;
                    d4 = this.c;
                    d3 = this.d;
                    d2 = this.e;
                    float f3 = 0.3f;
                    for (float f4 = this.g * (0.7f + this.k.s.nextFloat() * 0.6f); f4 > 0.0f; f4 -= f3 * 0.75f) {
                        int n5;
                        int n6;
                        int n7 = qh.c(d4);
                        aji aji2 = this.k.a(n7, n6 = qh.c(d3), n5 = qh.c(d2));
                        if (aji2.o() != awt.a) {
                            float f5 = this.f != null ? this.f.a(this, this.k, n7, n6, n5, aji2) : aji2.a(this.f);
                            f4 -= (f5 + 0.3f) * f3;
                        }
                        if (f4 > 0.0f && (this.f == null || this.f.a(this, this.k, n7, n6, n5, aji2, f4))) {
                            hashSet.add(new agt(n7, n6, n5));
                        }
                        d4 += d5 * (double)f3;
                        d3 += d6 * (double)f3;
                        d2 += d7 * (double)f3;
                    }
                }
            }
        }
        this.h.addAll(hashSet);
        this.g *= 2.0f;
        n4 = qh.c(this.c - (double)this.g - 1.0);
        n3 = qh.c(this.c + (double)this.g + 1.0);
        n2 = qh.c(this.d - (double)this.g - 1.0);
        int n8 = qh.c(this.d + (double)this.g + 1.0);
        int n9 = qh.c(this.e - (double)this.g - 1.0);
        int n10 = qh.c(this.e + (double)this.g + 1.0);
        List list = this.k.b(this.f, azt.a(n4, n2, n9, n3, n8, n10));
        azw azw2 = azw.a(this.c, this.d, this.e);
        for (int i2 = 0; i2 < list.size(); ++i2) {
            double d9;
            sa sa2 = (sa)list.get(i2);
            double d10 = sa2.f(this.c, this.d, this.e) / (double)this.g;
            if (!(d10 <= 1.0) || (d9 = (double)qh.a((d4 = sa2.s - this.c) * d4 + (d3 = sa2.t + (double)sa2.g() - this.d) * d3 + (d2 = sa2.u - this.e) * d2)) == 0.0) continue;
            d4 /= d9;
            d3 /= d9;
            d2 /= d9;
            double d11 = this.k.a(azw2, sa2.C);
            double d12 = (1.0 - d10) * d11;
            sa2.a(ro.a(this), (float)((int)((d12 * d12 + d12) / 2.0 * 8.0 * (double)this.g + 1.0)));
            double d13 = agi.a(sa2, d12);
            sa2.v += d4 * d13;
            sa2.w += d3 * d13;
            sa2.x += d2 * d13;
            if (!(sa2 instanceof yz)) continue;
            this.l.put((yz)sa2, azw.a(d4 * d12, d3 * d12, d2 * d12));
        }
        this.g = f2;
    }

    public void a(boolean bl2) {
        aji aji2;
        int n2;
        int n3;
        int n4;
        this.k.a(this.c, this.d, this.e, "random.explode", 4.0f, (1.0f + (this.k.s.nextFloat() - this.k.s.nextFloat()) * 0.2f) * 0.7f);
        if (this.g < 2.0f || !this.b) {
            this.k.a("largeexplode", this.c, this.d, this.e, 1.0, 0.0, 0.0);
        } else {
            this.k.a("hugeexplosion", this.c, this.d, this.e, 1.0, 0.0, 0.0);
        }
        if (this.b) {
            for (agt agt2 : this.h) {
                n4 = agt2.a;
                n3 = agt2.b;
                n2 = agt2.c;
                aji2 = this.k.a(n4, n3, n2);
                if (bl2) {
                    double d2 = (float)n4 + this.k.s.nextFloat();
                    double d3 = (float)n3 + this.k.s.nextFloat();
                    double d4 = (float)n2 + this.k.s.nextFloat();
                    double d5 = d2 - this.c;
                    double d6 = d3 - this.d;
                    double d7 = d4 - this.e;
                    double d8 = qh.a(d5 * d5 + d6 * d6 + d7 * d7);
                    d5 /= d8;
                    d6 /= d8;
                    d7 /= d8;
                    double d9 = 0.5 / (d8 / (double)this.g + 0.1);
                    this.k.a("explode", (d2 + this.c * 1.0) / 2.0, (d3 + this.d * 1.0) / 2.0, (d4 + this.e * 1.0) / 2.0, d5 *= (d9 *= (double)(this.k.s.nextFloat() * this.k.s.nextFloat() + 0.3f)), d6 *= d9, d7 *= d9);
                    this.k.a("smoke", d2, d3, d4, d5, d6, d7);
                }
                if (aji2.o() == awt.a) continue;
                if (aji2.a(this)) {
                    aji2.a(this.k, n4, n3, n2, this.k.e(n4, n3, n2), 1.0f / this.g, 0);
                }
                this.k.d(n4, n3, n2, ajn.a, 0, 3);
                aji2.a(this.k, n4, n3, n2, this);
            }
        }
        if (this.a) {
            for (agt agt2 : this.h) {
                n4 = agt2.a;
                n3 = agt2.b;
                n2 = agt2.c;
                aji2 = this.k.a(n4, n3, n2);
                aji aji3 = this.k.a(n4, n3 - 1, n2);
                if (aji2.o() != awt.a || !aji3.j() || this.j.nextInt(3) != 0) continue;
                this.k.b(n4, n3, n2, ajn.ab);
            }
        }
    }

    public Map b() {
        return this.l;
    }

    public sv c() {
        if (this.f == null) {
            return null;
        }
        if (this.f instanceof xw) {
            return ((xw)this.f).e();
        }
        if (this.f instanceof sv) {
            return (sv)this.f;
        }
        return null;
    }
}

