/*
 * Decompiled with CFR 0.152.
 */
public class ut
extends ui {
    private td a;
    private double b;
    private double c;
    private double d;
    private double e;

    public ut(td td2, double d2) {
        this.a = td2;
        this.e = d2;
        this.a(1);
    }

    @Override
    public boolean a() {
        if (this.a.bU()) {
            return false;
        }
        r r2 = this.a.bV();
        azw azw2 = vx.a(this.a, 16, 7, azw.a(r2.a, r2.b, r2.c));
        if (azw2 == null) {
            return false;
        }
        this.b = azw2.a;
        this.c = azw2.b;
        this.d = azw2.c;
        return true;
    }

    @Override
    public boolean b() {
        return !this.a.m().g();
    }

    @Override
    public void c() {
        this.a.m().a(this.b, this.c, this.d, this.e);
    }
}

