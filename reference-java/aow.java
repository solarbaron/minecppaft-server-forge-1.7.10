/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;

public class aow
extends aor
implements rb {
    private add[] p = new add[36];
    public boolean a;
    public aow i;
    public aow j;
    public aow k;
    public aow l;
    public float m;
    public float n;
    public int o;
    private int q;
    private int r = -1;
    private String s;

    @Override
    public int a() {
        return 27;
    }

    @Override
    public add a(int n2) {
        return this.p[n2];
    }

    @Override
    public add a(int n2, int n3) {
        if (this.p[n2] != null) {
            if (this.p[n2].b <= n3) {
                add add2 = this.p[n2];
                this.p[n2] = null;
                this.e();
                return add2;
            }
            add add3 = this.p[n2].a(n3);
            if (this.p[n2].b == 0) {
                this.p[n2] = null;
            }
            this.e();
            return add3;
        }
        return null;
    }

    @Override
    public add a_(int n2) {
        if (this.p[n2] != null) {
            add add2 = this.p[n2];
            this.p[n2] = null;
            return add2;
        }
        return null;
    }

    @Override
    public void a(int n2, add add2) {
        this.p[n2] = add2;
        if (add2 != null && add2.b > this.d()) {
            add2.b = this.d();
        }
        this.e();
    }

    @Override
    public String b() {
        return this.k_() ? this.s : "container.chest";
    }

    @Override
    public boolean k_() {
        return this.s != null && this.s.length() > 0;
    }

    public void a(String string) {
        this.s = string;
    }

    @Override
    public void a(dh dh2) {
        super.a(dh2);
        dq dq2 = dh2.c("Items", 10);
        this.p = new add[this.a()];
        if (dh2.b("CustomName", 8)) {
            this.s = dh2.j("CustomName");
        }
        for (int i2 = 0; i2 < dq2.c(); ++i2) {
            dh dh3 = dq2.b(i2);
            int n2 = dh3.d("Slot") & 0xFF;
            if (n2 < 0 || n2 >= this.p.length) continue;
            this.p[n2] = add.a(dh3);
        }
    }

    @Override
    public void b(dh dh2) {
        super.b(dh2);
        dq dq2 = new dq();
        for (int i2 = 0; i2 < this.p.length; ++i2) {
            if (this.p[i2] == null) continue;
            dh dh3 = new dh();
            dh3.a("Slot", (byte)i2);
            this.p[i2].b(dh3);
            dq2.a(dh3);
        }
        dh2.a("Items", dq2);
        if (this.k_()) {
            dh2.a("CustomName", this.s);
        }
    }

    @Override
    public int d() {
        return 64;
    }

    @Override
    public boolean a(yz yz2) {
        if (this.b.o(this.c, this.d, this.e) != this) {
            return false;
        }
        return !(yz2.e((double)this.c + 0.5, (double)this.d + 0.5, (double)this.e + 0.5) > 64.0);
    }

    @Override
    public void u() {
        super.u();
        this.a = false;
    }

    private void a(aow aow2, int n2) {
        if (aow2.r()) {
            this.a = false;
        } else if (this.a) {
            switch (n2) {
                case 2: {
                    if (this.i == aow2) break;
                    this.a = false;
                    break;
                }
                case 0: {
                    if (this.l == aow2) break;
                    this.a = false;
                    break;
                }
                case 3: {
                    if (this.j == aow2) break;
                    this.a = false;
                    break;
                }
                case 1: {
                    if (this.k == aow2) break;
                    this.a = false;
                }
            }
        }
    }

    public void i() {
        if (this.a) {
            return;
        }
        this.a = true;
        this.i = null;
        this.j = null;
        this.k = null;
        this.l = null;
        if (this.a(this.c - 1, this.d, this.e)) {
            this.k = (aow)this.b.o(this.c - 1, this.d, this.e);
        }
        if (this.a(this.c + 1, this.d, this.e)) {
            this.j = (aow)this.b.o(this.c + 1, this.d, this.e);
        }
        if (this.a(this.c, this.d, this.e - 1)) {
            this.i = (aow)this.b.o(this.c, this.d, this.e - 1);
        }
        if (this.a(this.c, this.d, this.e + 1)) {
            this.l = (aow)this.b.o(this.c, this.d, this.e + 1);
        }
        if (this.i != null) {
            this.i.a(this, 0);
        }
        if (this.l != null) {
            this.l.a(this, 2);
        }
        if (this.j != null) {
            this.j.a(this, 1);
        }
        if (this.k != null) {
            this.k.a(this, 3);
        }
    }

    private boolean a(int n2, int n3, int n4) {
        if (this.b == null) {
            return false;
        }
        aji aji2 = this.b.a(n2, n3, n4);
        return aji2 instanceof ajx && ((ajx)aji2).a == this.j();
    }

    @Override
    public void h() {
        float f2;
        super.h();
        this.i();
        ++this.q;
        if (!this.b.E && this.o != 0 && (this.q + this.c + this.d + this.e) % 200 == 0) {
            this.o = 0;
            f2 = 5.0f;
            List list = this.b.a(yz.class, azt.a((float)this.c - f2, (float)this.d - f2, (float)this.e - f2, (float)(this.c + 1) + f2, (float)(this.d + 1) + f2, (float)(this.e + 1) + f2));
            for (yz yz2 : list) {
                rb rb2;
                if (!(yz2.bo instanceof aad) || (rb2 = ((aad)yz2.bo).e()) != this && (!(rb2 instanceof ra) || !((ra)rb2).a(this))) continue;
                ++this.o;
            }
        }
        this.n = this.m;
        f2 = 0.1f;
        if (this.o > 0 && this.m == 0.0f && this.i == null && this.k == null) {
            double d2 = (double)this.c + 0.5;
            double d3 = (double)this.e + 0.5;
            if (this.l != null) {
                d3 += 0.5;
            }
            if (this.j != null) {
                d2 += 0.5;
            }
            this.b.a(d2, (double)this.d + 0.5, d3, "random.chestopen", 0.5f, this.b.s.nextFloat() * 0.1f + 0.9f);
        }
        if (this.o == 0 && this.m > 0.0f || this.o > 0 && this.m < 1.0f) {
            float f3;
            float f4 = this.m;
            this.m = this.o > 0 ? (this.m += f2) : (this.m -= f2);
            if (this.m > 1.0f) {
                this.m = 1.0f;
            }
            if (this.m < (f3 = 0.5f) && f4 >= f3 && this.i == null && this.k == null) {
                double d4 = (double)this.c + 0.5;
                double d5 = (double)this.e + 0.5;
                if (this.l != null) {
                    d5 += 0.5;
                }
                if (this.j != null) {
                    d4 += 0.5;
                }
                this.b.a(d4, (double)this.d + 0.5, d5, "random.chestclosed", 0.5f, this.b.s.nextFloat() * 0.1f + 0.9f);
            }
            if (this.m < 0.0f) {
                this.m = 0.0f;
            }
        }
    }

    @Override
    public boolean c(int n2, int n3) {
        if (n2 == 1) {
            this.o = n3;
            return true;
        }
        return super.c(n2, n3);
    }

    @Override
    public void f() {
        if (this.o < 0) {
            this.o = 0;
        }
        ++this.o;
        this.b.c(this.c, this.d, this.e, this.q(), 1, this.o);
        this.b.d(this.c, this.d, this.e, this.q());
        this.b.d(this.c, this.d - 1, this.e, this.q());
    }

    @Override
    public void l_() {
        if (this.q() instanceof ajx) {
            --this.o;
            this.b.c(this.c, this.d, this.e, this.q(), 1, this.o);
            this.b.d(this.c, this.d, this.e, this.q());
            this.b.d(this.c, this.d - 1, this.e, this.q());
        }
    }

    @Override
    public boolean b(int n2, add add2) {
        return true;
    }

    @Override
    public void s() {
        super.s();
        this.u();
        this.i();
    }

    public int j() {
        if (this.r == -1) {
            if (this.b != null && this.q() instanceof ajx) {
                this.r = ((ajx)this.q()).a;
            } else {
                return 0;
            }
        }
        return this.r;
    }
}

