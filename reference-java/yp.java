/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.UUID;

public class yp
extends yg
implements yi {
    private static final UUID bp = UUID.fromString("5CD17E52-A79A-43D3-A529-90FDE04B181E");
    private static final tj bq = new tj(bp, "Drinking speed penalty", -0.25, 0).a(false);
    private static final adb[] br = new adb[]{ade.aO, ade.aT, ade.ax, ade.bp, ade.bo, ade.H, ade.y, ade.y};
    private int bs;

    public yp(ahb ahb2) {
        super(ahb2);
        this.c.a(1, new uf(this));
        this.c.a(2, new vd(this, 1.0, 60, 10.0f));
        this.c.a(2, new vc(this, 1.0));
        this.c.a(3, new un(this, yz.class, 8.0f));
        this.c.a(3, new vb(this));
        this.d.a(1, new vn(this, false));
        this.d.a(2, new vo(this, yz.class, 0, true));
    }

    @Override
    protected void c() {
        super.c();
        this.z().a(21, (Object)0);
    }

    @Override
    protected String t() {
        return "mob.witch.idle";
    }

    @Override
    protected String aT() {
        return "mob.witch.hurt";
    }

    @Override
    protected String aU() {
        return "mob.witch.death";
    }

    public void a(boolean bl2) {
        this.z().b(21, bl2 ? (byte)1 : 0);
    }

    public boolean bZ() {
        return this.z().a(21) == 1;
    }

    @Override
    protected void aD() {
        super.aD();
        this.a(yj.a).a(26.0);
        this.a(yj.d).a(0.25);
    }

    @Override
    public boolean bk() {
        return true;
    }

    @Override
    public void e() {
        if (!this.o.E) {
            if (this.bZ()) {
                if (this.bs-- <= 0) {
                    List list;
                    this.a(false);
                    add add2 = this.be();
                    this.c(0, null);
                    if (add2 != null && add2.b() == ade.bn && (list = ade.bn.g(add2)) != null) {
                        for (rw rw2 : list) {
                            this.c(new rw(rw2));
                        }
                    }
                    this.a(yj.d).b(bq);
                }
            } else {
                int n2 = -1;
                if (this.Z.nextFloat() < 0.15f && this.a(awt.h) && !this.a(rv.o)) {
                    n2 = 8237;
                } else if (this.Z.nextFloat() < 0.15f && this.al() && !this.a(rv.n)) {
                    n2 = 16307;
                } else if (this.Z.nextFloat() < 0.05f && this.aS() < this.aY()) {
                    n2 = 16341;
                } else if (this.Z.nextFloat() < 0.25f && this.o() != null && !this.a(rv.c) && this.o().f(this) > 121.0) {
                    n2 = 16274;
                } else if (this.Z.nextFloat() < 0.25f && this.o() != null && !this.a(rv.c) && this.o().f(this) > 121.0) {
                    n2 = 16274;
                }
                if (n2 > -1) {
                    this.c(0, new add(ade.bn, 1, n2));
                    this.bs = this.be().n();
                    this.a(true);
                    ti ti2 = this.a(yj.d);
                    ti2.b(bq);
                    ti2.a(bq);
                }
            }
            if (this.Z.nextFloat() < 7.5E-4f) {
                this.o.a((sa)this, (byte)15);
            }
        }
        super.e();
    }

    @Override
    protected float c(ro ro2, float f2) {
        f2 = super.c(ro2, f2);
        if (ro2.j() == this) {
            f2 = 0.0f;
        }
        if (ro2.s()) {
            f2 = (float)((double)f2 * 0.15);
        }
        return f2;
    }

    @Override
    protected void b(boolean bl2, int n2) {
        int n3 = this.Z.nextInt(3) + 1;
        for (int i2 = 0; i2 < n3; ++i2) {
            int n4 = this.Z.nextInt(3);
            adb adb2 = br[this.Z.nextInt(br.length)];
            if (n2 > 0) {
                n4 += this.Z.nextInt(n2 + 1);
            }
            for (int i3 = 0; i3 < n4; ++i3) {
                this.a(adb2, 1);
            }
        }
    }

    @Override
    public void a(sv sv2, float f2) {
        if (this.bZ()) {
            return;
        }
        zo zo2 = new zo(this.o, (sv)this, 32732);
        zo2.z -= -20.0f;
        double d2 = sv2.s + sv2.v - this.s;
        double d3 = sv2.t + (double)sv2.g() - (double)1.1f - this.t;
        double d4 = sv2.u + sv2.x - this.u;
        float f3 = qh.a(d2 * d2 + d4 * d4);
        if (f3 >= 8.0f && !sv2.a(rv.d)) {
            zo2.a(32698);
        } else if (sv2.aS() >= 8.0f && !sv2.a(rv.u)) {
            zo2.a(32660);
        } else if (f3 <= 3.0f && !sv2.a(rv.t) && this.Z.nextFloat() < 0.25f) {
            zo2.a(32696);
        }
        zo2.c(d2, d3 + (double)(f3 * 0.2f), d4, 0.75f, 8.0f);
        this.o.d(zo2);
    }
}

