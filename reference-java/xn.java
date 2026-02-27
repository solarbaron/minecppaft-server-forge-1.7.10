/*
 * Decompiled with CFR 0.152.
 */
public class xn
extends xl {
    private final agp a = new xo(this);
    private int b = 0;

    public xn(ahb ahb2) {
        super(ahb2);
    }

    public xn(ahb ahb2, double d2, double d3, double d4) {
        super(ahb2, d2, d3, d4);
    }

    @Override
    protected void c() {
        super.c();
        this.z().a(23, "");
        this.z().a(24, "");
    }

    @Override
    protected void a(dh dh2) {
        super.a(dh2);
        this.a.b(dh2);
        this.z().b(23, this.e().i());
        this.z().b(24, fk.a(this.e().h()));
    }

    @Override
    protected void b(dh dh2) {
        super.b(dh2);
        this.a.a(dh2);
    }

    @Override
    public int m() {
        return 6;
    }

    @Override
    public aji o() {
        return ajn.bI;
    }

    public agp e() {
        return this.a;
    }

    @Override
    public void a(int n2, int n3, int n4, boolean bl2) {
        if (bl2 && this.aa - this.b >= 4) {
            this.e().a(this.o);
            this.b = this.aa;
        }
    }

    @Override
    public boolean c(yz yz2) {
        if (this.o.E) {
            yz2.a(this.e());
        }
        return super.c(yz2);
    }

    @Override
    public void i(int n2) {
        super.i(n2);
        if (n2 == 24) {
            try {
                this.a.b(fk.a(this.z().e(24)));
            }
            catch (Throwable throwable) {}
        } else if (n2 == 23) {
            this.a.a(this.z().e(23));
        }
    }
}

