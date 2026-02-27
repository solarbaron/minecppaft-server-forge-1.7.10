/*
 * Decompiled with CFR 0.152.
 */
public abstract class wu
extends td
implements ry {
    public wu(ahb ahb2) {
        super(ahb2);
    }

    @Override
    public boolean aE() {
        return true;
    }

    @Override
    public boolean by() {
        return this.o.b(this.C);
    }

    @Override
    public int q() {
        return 120;
    }

    @Override
    protected boolean v() {
        return true;
    }

    @Override
    protected int e(yz yz2) {
        return 1 + this.o.s.nextInt(3);
    }

    @Override
    public void C() {
        int n2 = this.ar();
        super.C();
        if (this.Z() && !this.M()) {
            this.h(--n2);
            if (this.ar() == -20) {
                this.h(0);
                this.a(ro.e, 2.0f);
            }
        } else {
            this.h(300);
        }
    }
}

