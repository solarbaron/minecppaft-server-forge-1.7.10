/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.UUID;

public class yh
extends yq {
    private static final UUID bq = UUID.fromString("49455A49-7EC5-45BA-B886-3B90B23A1718");
    private static final tj br = new tj(bq, "Attacking speed boost", 0.45, 0).a(false);
    private int bs;
    private int bt;
    private sa bu;

    public yh(ahb ahb2) {
        super(ahb2);
        this.ae = true;
    }

    @Override
    protected void aD() {
        super.aD();
        this.a(bp).a(0.0);
        this.a(yj.d).a(0.5);
        this.a(yj.e).a(5.0);
    }

    @Override
    protected boolean bk() {
        return false;
    }

    @Override
    public void h() {
        if (this.bu != this.bm && !this.o.E) {
            ti ti2 = this.a(yj.d);
            ti2.b(br);
            if (this.bm != null) {
                ti2.a(br);
            }
        }
        this.bu = this.bm;
        if (this.bt > 0 && --this.bt == 0) {
            this.a("mob.zombiepig.zpigangry", this.bf() * 2.0f, ((this.Z.nextFloat() - this.Z.nextFloat()) * 0.2f + 1.0f) * 1.8f);
        }
        super.h();
    }

    @Override
    public boolean by() {
        return this.o.r != rd.a && this.o.b(this.C) && this.o.a((sa)this, this.C).isEmpty() && !this.o.d(this.C);
    }

    @Override
    public void b(dh dh2) {
        super.b(dh2);
        dh2.a("Anger", (short)this.bs);
    }

    @Override
    public void a(dh dh2) {
        super.a(dh2);
        this.bs = dh2.e("Anger");
    }

    @Override
    protected sa bR() {
        if (this.bs == 0) {
            return null;
        }
        return super.bR();
    }

    @Override
    public boolean a(ro ro2, float f2) {
        if (this.aw()) {
            return false;
        }
        sa sa2 = ro2.j();
        if (sa2 instanceof yz) {
            List list = this.o.b((sa)this, this.C.b(32.0, 32.0, 32.0));
            for (int i2 = 0; i2 < list.size(); ++i2) {
                sa sa3 = (sa)list.get(i2);
                if (!(sa3 instanceof yh)) continue;
                yh yh2 = (yh)sa3;
                yh2.c(sa2);
            }
            this.c(sa2);
        }
        return super.a(ro2, f2);
    }

    private void c(sa sa2) {
        this.bm = sa2;
        this.bs = 400 + this.Z.nextInt(400);
        this.bt = this.Z.nextInt(40);
    }

    @Override
    protected String t() {
        return "mob.zombiepig.zpig";
    }

    @Override
    protected String aT() {
        return "mob.zombiepig.zpighurt";
    }

    @Override
    protected String aU() {
        return "mob.zombiepig.zpigdeath";
    }

    @Override
    protected void b(boolean bl2, int n2) {
        int n3;
        int n4 = this.Z.nextInt(2 + n2);
        for (n3 = 0; n3 < n4; ++n3) {
            this.a(ade.bh, 1);
        }
        n4 = this.Z.nextInt(2 + n2);
        for (n3 = 0; n3 < n4; ++n3) {
            this.a(ade.bl, 1);
        }
    }

    @Override
    public boolean a(yz yz2) {
        return false;
    }

    @Override
    protected void n(int n2) {
        this.a(ade.k, 1);
    }

    @Override
    protected void bC() {
        this.c(0, new add(ade.B));
    }

    @Override
    public sy a(sy sy2) {
        super.a(sy2);
        this.j(false);
        return sy2;
    }
}

