/*
 * Decompiled with CFR 0.152.
 */
public class vg
extends ui {
    private wi a;
    private double b;
    private double c;
    private double d;
    private double e;

    public vg(wi wi2, double d2) {
        this.a = wi2;
        this.b = d2;
        this.a(1);
    }

    @Override
    public boolean a() {
        if (this.a.cc() || this.a.l == null) {
            return false;
        }
        azw azw2 = vx.a(this.a, 5, 4);
        if (azw2 == null) {
            return false;
        }
        this.c = azw2.a;
        this.d = azw2.b;
        this.e = azw2.c;
        return true;
    }

    @Override
    public void c() {
        this.a.m().a(this.c, this.d, this.e, this.b);
    }

    @Override
    public boolean b() {
        return !this.a.m().g() && this.a.l != null;
    }

    @Override
    public void e() {
        if (this.a.aI().nextInt(50) == 0) {
            if (this.a.l instanceof yz) {
                int n2 = this.a.cq();
                int n3 = this.a.cw();
                if (n3 > 0 && this.a.aI().nextInt(n3) < n2) {
                    this.a.h((yz)this.a.l);
                    this.a.o.a((sa)this.a, (byte)7);
                    return;
                }
                this.a.v(5);
            }
            this.a.l.a((sa)null);
            this.a.l = null;
            this.a.cJ();
            this.a.o.a((sa)this.a, (byte)6);
        }
    }
}

