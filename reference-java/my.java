/*
 * Decompiled with CFR 0.152.
 */
import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class my {
    private static final Logger p = LogManager.getLogger();
    public sa a;
    public int b;
    public int c;
    public int d;
    public int e;
    public int f;
    public int g;
    public int h;
    public int i;
    public double j;
    public double k;
    public double l;
    public int m;
    private double q;
    private double r;
    private double s;
    private boolean t;
    private boolean u;
    private int v;
    private sa w;
    private boolean x;
    public boolean n;
    public Set o = new HashSet();

    public my(sa sa2, int n2, int n3, boolean bl2) {
        this.a = sa2;
        this.b = n2;
        this.c = n3;
        this.u = bl2;
        this.d = qh.c(sa2.s * 32.0);
        this.e = qh.c(sa2.t * 32.0);
        this.f = qh.c(sa2.u * 32.0);
        this.g = qh.d(sa2.y * 256.0f / 360.0f);
        this.h = qh.d(sa2.z * 256.0f / 360.0f);
        this.i = qh.d(sa2.au() * 256.0f / 360.0f);
    }

    public boolean equals(Object object) {
        if (object instanceof my) {
            return ((my)object).a.y() == this.a.y();
        }
        return false;
    }

    public int hashCode() {
        return this.a.y();
    }

    public void a(List list) {
        this.n = false;
        if (!this.t || this.a.e(this.q, this.r, this.s) > 16.0) {
            this.q = this.a.s;
            this.r = this.a.t;
            this.s = this.a.u;
            this.t = true;
            this.n = true;
            this.b(list);
        }
        if (this.w != this.a.m || this.a.m != null && this.m % 60 == 0) {
            this.w = this.a.m;
            this.a(new hx(0, this.a, this.a.m));
        }
        if (this.a instanceof st && this.m % 10 == 0) {
            st st2 = (st)this.a;
            add add2 = st2.j();
            if (add2 != null && add2.b() instanceof adh) {
                ayi ayi2 = ade.aY.a(add2, this.a.o);
                for (yz yz2 : list) {
                    mw mw2 = (mw)yz2;
                    ayi2.a(mw2, add2);
                    ft ft2 = ade.aY.c(add2, this.a.o, mw2);
                    if (ft2 == null) continue;
                    mw2.a.a(ft2);
                }
            }
            this.b();
        } else if (this.m % this.c == 0 || this.a.al || this.a.z().a()) {
            int n2;
            if (this.a.m == null) {
                double d2;
                double d3;
                double d4;
                double d5;
                double d6;
                boolean bl2;
                ++this.v;
                n2 = this.a.as.a(this.a.s);
                int n3 = qh.c(this.a.t * 32.0);
                int n4 = this.a.as.a(this.a.u);
                int n5 = qh.d(this.a.y * 256.0f / 360.0f);
                int n6 = qh.d(this.a.z * 256.0f / 360.0f);
                int n7 = n2 - this.d;
                int n8 = n3 - this.e;
                int n9 = n4 - this.f;
                ft ft3 = null;
                boolean bl3 = Math.abs(n7) >= 4 || Math.abs(n8) >= 4 || Math.abs(n9) >= 4 || this.m % 60 == 0;
                boolean bl4 = bl2 = Math.abs(n5 - this.g) >= 4 || Math.abs(n6 - this.h) >= 4;
                if (this.m > 0 || this.a instanceof zc) {
                    if (n7 < -128 || n7 >= 128 || n8 < -128 || n8 >= 128 || n9 < -128 || n9 >= 128 || this.v > 400 || this.x) {
                        this.v = 0;
                        ft3 = new ik(this.a.y(), n2, n3, n4, (byte)n5, (byte)n6);
                    } else if (bl3 && bl2) {
                        ft3 = new hh(this.a.y(), (byte)n7, (byte)n8, (byte)n9, (byte)n5, (byte)n6);
                    } else if (bl3) {
                        ft3 = new hg(this.a.y(), (byte)n7, (byte)n8, (byte)n9);
                    } else if (bl2) {
                        ft3 = new hi(this.a.y(), (byte)n5, (byte)n6);
                    }
                }
                if (this.u && ((d6 = (d5 = this.a.v - this.j) * d5 + (d4 = this.a.w - this.k) * d4 + (d3 = this.a.x - this.l) * d3) > (d2 = 0.02) * d2 || d6 > 0.0 && this.a.v == 0.0 && this.a.w == 0.0 && this.a.x == 0.0)) {
                    this.j = this.a.v;
                    this.k = this.a.w;
                    this.l = this.a.x;
                    this.a(new hy(this.a.y(), this.j, this.k, this.l));
                }
                if (ft3 != null) {
                    this.a(ft3);
                }
                this.b();
                if (bl3) {
                    this.d = n2;
                    this.e = n3;
                    this.f = n4;
                }
                if (bl2) {
                    this.g = n5;
                    this.h = n6;
                }
                this.x = false;
            } else {
                boolean bl5;
                n2 = qh.d(this.a.y * 256.0f / 360.0f);
                int n10 = qh.d(this.a.z * 256.0f / 360.0f);
                boolean bl6 = bl5 = Math.abs(n2 - this.g) >= 4 || Math.abs(n10 - this.h) >= 4;
                if (bl5) {
                    this.a(new hi(this.a.y(), (byte)n2, (byte)n10));
                    this.g = n2;
                    this.h = n10;
                }
                this.d = this.a.as.a(this.a.s);
                this.e = qh.c(this.a.t * 32.0);
                this.f = this.a.as.a(this.a.u);
                this.b();
                this.x = true;
            }
            n2 = qh.d(this.a.au() * 256.0f / 360.0f);
            if (Math.abs(n2 - this.i) >= 4) {
                this.a(new ht(this.a, (byte)n2));
                this.i = n2;
            }
            this.a.al = false;
        }
        ++this.m;
        if (this.a.H) {
            this.b(new hy(this.a));
            this.a.H = false;
        }
    }

    private void b() {
        te te2 = this.a.z();
        if (te2.a()) {
            this.b(new hw(this.a.y(), te2, false));
        }
        if (this.a instanceof sv) {
            tq tq2 = (tq)((sv)this.a).bc();
            Set set = tq2.b();
            if (!set.isEmpty()) {
                this.b(new il(this.a.y(), set));
            }
            set.clear();
        }
    }

    public void a(ft ft2) {
        for (mw mw2 : this.o) {
            mw2.a.a(ft2);
        }
    }

    public void b(ft ft2) {
        this.a(ft2);
        if (this.a instanceof mw) {
            ((mw)this.a).a.a(ft2);
        }
    }

    public void a() {
        for (mw mw2 : this.o) {
            mw2.d(this.a);
        }
    }

    public void a(mw mw2) {
        if (this.o.contains(mw2)) {
            mw2.d(this.a);
            this.o.remove(mw2);
        }
    }

    public void b(mw mw2) {
        if (mw2 == this.a) {
            return;
        }
        double d2 = mw2.s - (double)(this.d / 32);
        double d3 = mw2.u - (double)(this.f / 32);
        if (d2 >= (double)(-this.b) && d2 <= (double)this.b && d3 >= (double)(-this.b) && d3 <= (double)this.b) {
            if (!this.o.contains(mw2) && (this.d(mw2) || this.a.n)) {
                Object object;
                Object object2;
                this.o.add(mw2);
                ft ft2 = this.c();
                mw2.a.a(ft2);
                if (!this.a.z().d()) {
                    mw2.a.a(new hw(this.a.y(), this.a.z(), true));
                }
                if (this.a instanceof sv && !(object2 = ((tq)(object = (tq)((sv)this.a).bc())).c()).isEmpty()) {
                    mw2.a.a(new il(this.a.y(), (Collection)object2));
                }
                this.j = this.a.v;
                this.k = this.a.w;
                this.l = this.a.x;
                if (this.u && !(ft2 instanceof fz)) {
                    mw2.a.a(new hy(this.a.y(), this.a.v, this.a.w, this.a.x));
                }
                if (this.a.m != null) {
                    mw2.a.a(new hx(0, this.a, this.a.m));
                }
                if (this.a instanceof sw && ((sw)this.a).bO() != null) {
                    mw2.a.a(new hx(1, this.a, ((sw)this.a).bO()));
                }
                if (this.a instanceof sv) {
                    for (int i2 = 0; i2 < 5; ++i2) {
                        object2 = ((sv)this.a).q(i2);
                        if (object2 == null) continue;
                        mw2.a.a(new hz(this.a.y(), i2, (add)object2));
                    }
                }
                if (this.a instanceof yz && ((yz)(object = (yz)this.a)).bm()) {
                    mw2.a.a(new hp((yz)object, qh.c(this.a.s), qh.c(this.a.t), qh.c(this.a.u)));
                }
                if (this.a instanceof sv) {
                    object = (sv)this.a;
                    for (rw rw2 : ((sv)object).aQ()) {
                        mw2.a.a(new in(this.a.y(), rw2));
                    }
                }
            }
        } else if (this.o.contains(mw2)) {
            this.o.remove(mw2);
            mw2.d(this.a);
        }
    }

    private boolean d(mw mw2) {
        return mw2.r().t().a(mw2, this.a.ah, this.a.aj);
    }

    public void b(List list) {
        for (int i2 = 0; i2 < list.size(); ++i2) {
            this.b((mw)list.get(i2));
        }
    }

    private ft c() {
        if (this.a.K) {
            p.warn("Fetching addPacket for removed entity");
        }
        if (this.a instanceof xk) {
            return new fw(this.a, 2, 1);
        }
        if (this.a instanceof mw) {
            return new gb((yz)this.a);
        }
        if (this.a instanceof xl) {
            xl xl2 = (xl)this.a;
            return new fw(this.a, 10, xl2.m());
        }
        if (this.a instanceof xi) {
            return new fw(this.a, 1);
        }
        if (this.a instanceof ry || this.a instanceof xa) {
            this.i = qh.d(this.a.au() * 256.0f / 360.0f);
            return new fz((sv)this.a);
        }
        if (this.a instanceof xe) {
            yz yz2 = ((xe)this.a).b;
            return new fw(this.a, 90, yz2 != null ? yz2.y() : this.a.y());
        }
        if (this.a instanceof zc) {
            sa sa2 = ((zc)this.a).c;
            return new fw(this.a, 60, sa2 != null ? sa2.y() : this.a.y());
        }
        if (this.a instanceof zj) {
            return new fw(this.a, 61);
        }
        if (this.a instanceof zo) {
            return new fw(this.a, 73, ((zo)this.a).k());
        }
        if (this.a instanceof zn) {
            return new fw(this.a, 75);
        }
        if (this.a instanceof zm) {
            return new fw(this.a, 65);
        }
        if (this.a instanceof zd) {
            return new fw(this.a, 72);
        }
        if (this.a instanceof zf) {
            return new fw(this.a, 76);
        }
        if (this.a instanceof ze) {
            ze ze2 = (ze)this.a;
            fw fw2 = null;
            int n2 = 63;
            if (this.a instanceof zi) {
                n2 = 64;
            } else if (this.a instanceof zp) {
                n2 = 66;
            }
            fw2 = ze2.a != null ? new fw(this.a, n2, ((ze)this.a).a.y()) : new fw(this.a, n2, 0);
            fw2.d((int)(ze2.b * 8000.0));
            fw2.e((int)(ze2.c * 8000.0));
            fw2.f((int)(ze2.d * 8000.0));
            return fw2;
        }
        if (this.a instanceof zl) {
            return new fw(this.a, 62);
        }
        if (this.a instanceof xw) {
            return new fw(this.a, 50);
        }
        if (this.a instanceof wz) {
            return new fw(this.a, 51);
        }
        if (this.a instanceof xj) {
            xj xj2 = (xj)this.a;
            return new fw(this.a, 70, aji.b(xj2.f()) | xj2.a << 16);
        }
        if (this.a instanceof tb) {
            return new ga((tb)this.a);
        }
        if (this.a instanceof st) {
            st st2 = (st)this.a;
            fw fw3 = new fw(this.a, 71, st2.a);
            fw3.a(qh.d(st2.b * 32));
            fw3.b(qh.d(st2.c * 32));
            fw3.c(qh.d(st2.d * 32));
            return fw3;
        }
        if (this.a instanceof su) {
            su su2 = (su)this.a;
            fw fw4 = new fw(this.a, 77);
            fw4.a(qh.d(su2.b * 32));
            fw4.b(qh.d(su2.c * 32));
            fw4.c(qh.d(su2.d * 32));
            return fw4;
        }
        if (this.a instanceof sq) {
            return new fx((sq)this.a);
        }
        throw new IllegalArgumentException("Don't know how to add " + this.a.getClass() + "!");
    }

    public void c(mw mw2) {
        if (this.o.contains(mw2)) {
            this.o.remove(mw2);
            mw2.d(this.a);
        }
    }
}

