/*
 * Decompiled with CFR 0.152.
 */
public class vl
extends ui {
    private yv a;

    public vl(yv yv2) {
        this.a = yv2;
        this.a(5);
    }

    @Override
    public boolean a() {
        if (!this.a.Z()) {
            return false;
        }
        if (this.a.M()) {
            return false;
        }
        if (!this.a.D) {
            return false;
        }
        if (this.a.H) {
            return false;
        }
        yz yz2 = this.a.b();
        if (yz2 == null) {
            return false;
        }
        if (this.a.f(yz2) > 16.0) {
            return false;
        }
        return yz2.bo instanceof zs;
    }

    @Override
    public void c() {
        this.a.m().h();
    }

    @Override
    public void d() {
        this.a.a_((yz)null);
    }
}

