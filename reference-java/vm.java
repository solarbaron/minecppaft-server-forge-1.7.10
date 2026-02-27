/*
 * Decompiled with CFR 0.152.
 */
public class vm
extends vu {
    wt a;
    sv b;

    public vm(wt wt2) {
        super(wt2, false, true);
        this.a = wt2;
        this.a(1);
    }

    @Override
    public boolean a() {
        vz vz2 = this.a.bZ();
        if (vz2 == null) {
            return false;
        }
        this.b = vz2.b(this.a);
        if (!this.a(this.b, false)) {
            if (this.c.aI().nextInt(20) == 0) {
                this.b = vz2.c(this.a);
                return this.a(this.b, false);
            }
            return false;
        }
        return true;
    }

    @Override
    public void c() {
        this.a.d(this.b);
        super.c();
    }
}

