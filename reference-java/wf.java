/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;

public abstract class wf
extends rx
implements ry {
    private int bp;
    private int bq;
    private yz br;

    public wf(ahb ahb2) {
        super(ahb2);
    }

    @Override
    protected void bp() {
        if (this.d() != 0) {
            this.bp = 0;
        }
        super.bp();
    }

    @Override
    public void e() {
        super.e();
        if (this.d() != 0) {
            this.bp = 0;
        }
        if (this.bp > 0) {
            --this.bp;
            String string = "heart";
            if (this.bp % 10 == 0) {
                double d2 = this.Z.nextGaussian() * 0.02;
                double d3 = this.Z.nextGaussian() * 0.02;
                double d4 = this.Z.nextGaussian() * 0.02;
                this.o.a(string, this.s + (double)(this.Z.nextFloat() * this.M * 2.0f) - (double)this.M, this.t + 0.5 + (double)(this.Z.nextFloat() * this.N), this.u + (double)(this.Z.nextFloat() * this.M * 2.0f) - (double)this.M, d2, d3, d4);
            }
        } else {
            this.bq = 0;
        }
    }

    @Override
    protected void a(sa sa2, float f2) {
        if (sa2 instanceof yz) {
            yz yz2;
            if (f2 < 3.0f) {
                double d2 = sa2.s - this.s;
                double d3 = sa2.u - this.u;
                this.y = (float)(Math.atan2(d3, d2) * 180.0 / 3.1415927410125732) - 90.0f;
                this.bn = true;
            }
            if ((yz2 = (yz)sa2).bF() == null || !this.c(yz2.bF())) {
                this.bm = null;
            }
        } else if (sa2 instanceof wf) {
            wf wf2 = (wf)sa2;
            if (this.d() > 0 && wf2.d() < 0) {
                if ((double)f2 < 2.5) {
                    this.bn = true;
                }
            } else if (this.bp > 0 && wf2.bp > 0) {
                if (wf2.bm == null) {
                    wf2.bm = this;
                }
                if (wf2.bm == this && (double)f2 < 3.5) {
                    ++wf2.bp;
                    ++this.bp;
                    ++this.bq;
                    if (this.bq % 4 == 0) {
                        this.o.a("heart", this.s + (double)(this.Z.nextFloat() * this.M * 2.0f) - (double)this.M, this.t + 0.5 + (double)(this.Z.nextFloat() * this.N), this.u + (double)(this.Z.nextFloat() * this.M * 2.0f) - (double)this.M, 0.0, 0.0, 0.0);
                    }
                    if (this.bq == 60) {
                        this.b((wf)sa2);
                    }
                } else {
                    this.bq = 0;
                }
            } else {
                this.bq = 0;
                this.bm = null;
            }
        }
    }

    private void b(wf wf2) {
        rx rx2 = this.a((rx)wf2);
        if (rx2 != null) {
            if (this.br == null && wf2.cd() != null) {
                this.br = wf2.cd();
            }
            if (this.br != null) {
                this.br.a(pp.x);
                if (this instanceof wh) {
                    this.br.a(pc.H);
                }
            }
            this.c(6000);
            wf2.c(6000);
            this.bp = 0;
            this.bq = 0;
            this.bm = null;
            wf2.bm = null;
            wf2.bq = 0;
            wf2.bp = 0;
            rx2.c(-24000);
            rx2.b(this.s, this.t, this.u, this.y, this.z);
            for (int i2 = 0; i2 < 7; ++i2) {
                double d2 = this.Z.nextGaussian() * 0.02;
                double d3 = this.Z.nextGaussian() * 0.02;
                double d4 = this.Z.nextGaussian() * 0.02;
                this.o.a("heart", this.s + (double)(this.Z.nextFloat() * this.M * 2.0f) - (double)this.M, this.t + 0.5 + (double)(this.Z.nextFloat() * this.N), this.u + (double)(this.Z.nextFloat() * this.M * 2.0f) - (double)this.M, d2, d3, d4);
            }
            this.o.d(rx2);
        }
    }

    @Override
    public boolean a(ro ro2, float f2) {
        ti ti2;
        if (this.aw()) {
            return false;
        }
        this.bo = 60;
        if (!this.bk() && (ti2 = this.a(yj.d)).a(h) == null) {
            ti2.a(i);
        }
        this.bm = null;
        this.bp = 0;
        return super.a(ro2, f2);
    }

    @Override
    public float a(int n2, int n3, int n4) {
        if (this.o.a(n2, n3 - 1, n4) == ajn.c) {
            return 10.0f;
        }
        return this.o.n(n2, n3, n4) - 0.5f;
    }

    @Override
    public void b(dh dh2) {
        super.b(dh2);
        dh2.a("InLove", this.bp);
    }

    @Override
    public void a(dh dh2) {
        super.a(dh2);
        this.bp = dh2.f("InLove");
    }

    @Override
    protected sa bR() {
        block5: {
            float f2;
            block6: {
                block4: {
                    if (this.bo > 0) {
                        return null;
                    }
                    f2 = 8.0f;
                    if (this.bp <= 0) break block4;
                    List list = this.o.a(this.getClass(), this.C.b(f2, f2, f2));
                    for (int i2 = 0; i2 < list.size(); ++i2) {
                        wf wf2 = (wf)list.get(i2);
                        if (wf2 == this || wf2.bp <= 0) continue;
                        return wf2;
                    }
                    break block5;
                }
                if (this.d() != 0) break block6;
                List list = this.o.a(yz.class, this.C.b(f2, f2, f2));
                for (int i3 = 0; i3 < list.size(); ++i3) {
                    yz yz2 = (yz)list.get(i3);
                    if (yz2.bF() == null || !this.c(yz2.bF())) continue;
                    return yz2;
                }
                break block5;
            }
            if (this.d() <= 0) break block5;
            List list = this.o.a(this.getClass(), this.C.b(f2, f2, f2));
            for (int i4 = 0; i4 < list.size(); ++i4) {
                wf wf3 = (wf)list.get(i4);
                if (wf3 == this || wf3.d() >= 0) continue;
                return wf3;
            }
        }
        return null;
    }

    @Override
    public boolean by() {
        int n2;
        int n3;
        int n4 = qh.c(this.s);
        return this.o.a(n4, (n3 = qh.c(this.C.b)) - 1, n2 = qh.c(this.u)) == ajn.c && this.o.j(n4, n3, n2) > 8 && super.by();
    }

    @Override
    public int q() {
        return 120;
    }

    @Override
    protected boolean v() {
        return false;
    }

    @Override
    protected int e(yz yz2) {
        return 1 + this.o.s.nextInt(3);
    }

    public boolean c(add add2) {
        return add2.b() == ade.O;
    }

    @Override
    public boolean a(yz yz2) {
        add add2 = yz2.bm.h();
        if (add2 != null && this.c(add2) && this.d() == 0 && this.bp <= 0) {
            if (!yz2.bE.d) {
                --add2.b;
                if (add2.b <= 0) {
                    yz2.bm.a(yz2.bm.c, null);
                }
            }
            this.f(yz2);
            return true;
        }
        return super.a(yz2);
    }

    public void f(yz yz2) {
        this.bp = 600;
        this.br = yz2;
        this.bm = null;
        this.o.a((sa)this, (byte)18);
    }

    public yz cd() {
        return this.br;
    }

    public boolean ce() {
        return this.bp > 0;
    }

    public void cf() {
        this.bp = 0;
    }

    public boolean a(wf wf2) {
        if (wf2 == this) {
            return false;
        }
        if (wf2.getClass() != this.getClass()) {
            return false;
        }
        return this.ce() && wf2.ce();
    }
}

