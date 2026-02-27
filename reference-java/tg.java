/*
 * Decompiled with CFR 0.152.
 */
import java.util.UUID;

public abstract class tg
extends wf
implements ta {
    protected vh bp = new vh(this);

    public tg(ahb ahb2) {
        super(ahb2);
    }

    @Override
    protected void c() {
        super.c();
        this.af.a(16, (Object)0);
        this.af.a(17, "");
    }

    @Override
    public void b(dh dh2) {
        super.b(dh2);
        if (this.b() == null) {
            dh2.a("OwnerUUID", "");
        } else {
            dh2.a("OwnerUUID", this.b());
        }
        dh2.a("Sitting", this.ca());
    }

    @Override
    public void a(dh dh2) {
        super.a(dh2);
        String string = "";
        if (dh2.b("OwnerUUID", 8)) {
            string = dh2.j("OwnerUUID");
        } else {
            String string2 = dh2.j("Owner");
            string = nz.a(string2);
        }
        if (string.length() > 0) {
            this.b(string);
            this.j(true);
        }
        this.bp.a(dh2.n("Sitting"));
        this.k(dh2.n("Sitting"));
    }

    protected void i(boolean bl2) {
        String string = "heart";
        if (!bl2) {
            string = "smoke";
        }
        for (int i2 = 0; i2 < 7; ++i2) {
            double d2 = this.Z.nextGaussian() * 0.02;
            double d3 = this.Z.nextGaussian() * 0.02;
            double d4 = this.Z.nextGaussian() * 0.02;
            this.o.a(string, this.s + (double)(this.Z.nextFloat() * this.M * 2.0f) - (double)this.M, this.t + 0.5 + (double)(this.Z.nextFloat() * this.N), this.u + (double)(this.Z.nextFloat() * this.M * 2.0f) - (double)this.M, d2, d3, d4);
        }
    }

    public boolean bZ() {
        return (this.af.a(16) & 4) != 0;
    }

    public void j(boolean bl2) {
        byte by2 = this.af.a(16);
        if (bl2) {
            this.af.b(16, (byte)(by2 | 4));
        } else {
            this.af.b(16, (byte)(by2 & 0xFFFFFFFB));
        }
    }

    public boolean ca() {
        return (this.af.a(16) & 1) != 0;
    }

    public void k(boolean bl2) {
        byte by2 = this.af.a(16);
        if (bl2) {
            this.af.b(16, (byte)(by2 | 1));
        } else {
            this.af.b(16, (byte)(by2 & 0xFFFFFFFE));
        }
    }

    @Override
    public String b() {
        return this.af.e(17);
    }

    public void b(String string) {
        this.af.b(17, string);
    }

    public sv cb() {
        try {
            UUID uUID = UUID.fromString(this.b());
            if (uUID == null) {
                return null;
            }
            return this.o.a(uUID);
        }
        catch (IllegalArgumentException illegalArgumentException) {
            return null;
        }
    }

    public boolean e(sv sv2) {
        return sv2 == this.cb();
    }

    public vh cc() {
        return this.bp;
    }

    public boolean a(sv sv2, sv sv3) {
        return true;
    }

    @Override
    public bae bt() {
        sv sv2;
        if (this.bZ() && (sv2 = this.cb()) != null) {
            return sv2.bt();
        }
        return super.bt();
    }

    @Override
    public boolean c(sv sv2) {
        if (this.bZ()) {
            sv sv3 = this.cb();
            if (sv2 == sv3) {
                return true;
            }
            if (sv3 != null) {
                return sv3.c(sv2);
            }
        }
        return super.c(sv2);
    }

    @Override
    public /* synthetic */ sa i_() {
        return this.cb();
    }
}

