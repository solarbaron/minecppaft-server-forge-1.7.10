/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;

public class xr
extends xp
implements aph {
    private boolean a = true;
    private int b = -1;

    public xr(ahb ahb2) {
        super(ahb2);
    }

    public xr(ahb ahb2, double d2, double d3, double d4) {
        super(ahb2, d2, d3, d4);
    }

    @Override
    public int m() {
        return 5;
    }

    @Override
    public aji o() {
        return ajn.bZ;
    }

    @Override
    public int s() {
        return 1;
    }

    @Override
    public int a() {
        return 5;
    }

    @Override
    public boolean c(yz yz2) {
        if (!this.o.E) {
            yz2.a(this);
        }
        return true;
    }

    @Override
    public void a(int n2, int n3, int n4, boolean bl2) {
        boolean bl3;
        boolean bl4 = bl3 = !bl2;
        if (bl3 != this.v()) {
            this.f(bl3);
        }
    }

    public boolean v() {
        return this.a;
    }

    public void f(boolean bl2) {
        this.a = bl2;
    }

    @Override
    public ahb w() {
        return this.o;
    }

    @Override
    public double x() {
        return this.s;
    }

    @Override
    public double aD() {
        return this.t;
    }

    @Override
    public double aE() {
        return this.u;
    }

    @Override
    public void h() {
        super.h();
        if (!this.o.E && this.Z() && this.v()) {
            --this.b;
            if (!this.aG()) {
                this.n(0);
                if (this.aF()) {
                    this.n(4);
                    this.e();
                }
            }
        }
    }

    public boolean aF() {
        if (api.a(this)) {
            return true;
        }
        List list = this.o.a(xk.class, this.C.b(0.25, 0.0, 0.25), sj.a);
        if (list.size() > 0) {
            api.a(this, (xk)list.get(0));
        }
        return false;
    }

    @Override
    public void a(ro ro2) {
        super.a(ro2);
        this.a(adb.a(ajn.bZ), 1, 0.0f);
    }

    @Override
    protected void b(dh dh2) {
        super.b(dh2);
        dh2.a("TransferCooldown", this.b);
    }

    @Override
    protected void a(dh dh2) {
        super.a(dh2);
        this.b = dh2.f("TransferCooldown");
    }

    public void n(int n2) {
        this.b = n2;
    }

    public boolean aG() {
        return this.b > 0;
    }
}

