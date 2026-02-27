/*
 * Decompiled with CFR 0.152.
 */
public class uz
extends ui {
    private td a;
    private double b;
    private double c;
    private double d;
    private double e;

    public uz(td td2, double d2) {
        this.a = td2;
        this.b = d2;
        this.a(1);
    }

    @Override
    public boolean a() {
        if (this.a.aJ() == null && !this.a.al()) {
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
        return !this.a.m().g();
    }
}

