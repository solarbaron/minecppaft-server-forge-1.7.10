/*
 * Decompiled with CFR 0.152.
 */
import java.util.Calendar;

public class we
extends wd {
    private r h;

    public we(ahb ahb2) {
        super(ahb2);
        this.a(0.5f, 0.9f);
        this.a(true);
    }

    @Override
    protected void c() {
        super.c();
        this.af.a(16, new Byte(0));
    }

    @Override
    protected float bf() {
        return 0.1f;
    }

    @Override
    protected float bg() {
        return super.bg() * 0.95f;
    }

    @Override
    protected String t() {
        if (this.bP() && this.Z.nextInt(4) != 0) {
            return null;
        }
        return "mob.bat.idle";
    }

    @Override
    protected String aT() {
        return "mob.bat.hurt";
    }

    @Override
    protected String aU() {
        return "mob.bat.death";
    }

    @Override
    public boolean S() {
        return false;
    }

    @Override
    protected void o(sa sa2) {
    }

    @Override
    protected void bo() {
    }

    @Override
    protected void aD() {
        super.aD();
        this.a(yj.a).a(6.0);
    }

    public boolean bP() {
        return (this.af.a(16) & 1) != 0;
    }

    public void a(boolean bl2) {
        byte by2 = this.af.a(16);
        if (bl2) {
            this.af.b(16, (byte)(by2 | 1));
        } else {
            this.af.b(16, (byte)(by2 & 0xFFFFFFFE));
        }
    }

    @Override
    protected boolean bk() {
        return true;
    }

    @Override
    public void h() {
        super.h();
        if (this.bP()) {
            this.x = 0.0;
            this.w = 0.0;
            this.v = 0.0;
            this.t = (double)qh.c(this.t) + 1.0 - (double)this.N;
        } else {
            this.w *= (double)0.6f;
        }
    }

    @Override
    protected void bn() {
        super.bn();
        if (this.bP()) {
            if (!this.o.a(qh.c(this.s), (int)this.t + 1, qh.c(this.u)).r()) {
                this.a(false);
                this.o.a(null, 1015, (int)this.s, (int)this.t, (int)this.u, 0);
            } else {
                if (this.Z.nextInt(200) == 0) {
                    this.aO = this.Z.nextInt(360);
                }
                if (this.o.a((sa)this, 4.0) != null) {
                    this.a(false);
                    this.o.a(null, 1015, (int)this.s, (int)this.t, (int)this.u, 0);
                }
            }
        } else {
            if (!(this.h == null || this.o.c(this.h.a, this.h.b, this.h.c) && this.h.b >= 1)) {
                this.h = null;
            }
            if (this.h == null || this.Z.nextInt(30) == 0 || this.h.e((int)this.s, (int)this.t, (int)this.u) < 4.0f) {
                this.h = new r((int)this.s + this.Z.nextInt(7) - this.Z.nextInt(7), (int)this.t + this.Z.nextInt(6) - 2, (int)this.u + this.Z.nextInt(7) - this.Z.nextInt(7));
            }
            double d2 = (double)this.h.a + 0.5 - this.s;
            double d3 = (double)this.h.b + 0.1 - this.t;
            double d4 = (double)this.h.c + 0.5 - this.u;
            this.v += (Math.signum(d2) * 0.5 - this.v) * (double)0.1f;
            this.w += (Math.signum(d3) * (double)0.7f - this.w) * (double)0.1f;
            this.x += (Math.signum(d4) * 0.5 - this.x) * (double)0.1f;
            float f2 = (float)(Math.atan2(this.x, this.v) * 180.0 / 3.1415927410125732) - 90.0f;
            float f3 = qh.g(f2 - this.y);
            this.be = 0.5f;
            this.y += f3;
            if (this.Z.nextInt(100) == 0 && this.o.a(qh.c(this.s), (int)this.t + 1, qh.c(this.u)).r()) {
                this.a(true);
            }
        }
    }

    @Override
    protected boolean g_() {
        return false;
    }

    @Override
    protected void b(float f2) {
    }

    @Override
    protected void a(double d2, boolean bl2) {
    }

    @Override
    public boolean az() {
        return true;
    }

    @Override
    public boolean a(ro ro2, float f2) {
        if (this.aw()) {
            return false;
        }
        if (!this.o.E && this.bP()) {
            this.a(false);
        }
        return super.a(ro2, f2);
    }

    @Override
    public void a(dh dh2) {
        super.a(dh2);
        this.af.b(16, dh2.d("BatFlags"));
    }

    @Override
    public void b(dh dh2) {
        super.b(dh2);
        dh2.a("BatFlags", this.af.a(16));
    }

    @Override
    public boolean by() {
        int n2 = qh.c(this.C.b);
        if (n2 >= 63) {
            return false;
        }
        int n3 = qh.c(this.s);
        int n4 = qh.c(this.u);
        int n5 = this.o.k(n3, n2, n4);
        int n6 = 4;
        Calendar calendar = this.o.V();
        if (calendar.get(2) + 1 == 10 && calendar.get(5) >= 20 || calendar.get(2) + 1 == 11 && calendar.get(5) <= 3) {
            n6 = 7;
        } else if (this.Z.nextBoolean()) {
            return false;
        }
        if (n5 > this.Z.nextInt(n6)) {
            return false;
        }
        return super.by();
    }
}

