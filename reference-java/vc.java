/*
 * Decompiled with CFR 0.152.
 */
public class vc
extends ui {
    private td a;
    private double b;
    private double c;
    private double d;
    private double e;

    public vc(td td2, double d2) {
        this.a = td2;
        this.e = d2;
        this.a(1);
    }

    @Override
    public boolean a() {
        if (this.a.aN() >= 100) {
            return false;
        }
        if (this.a.aI().nextInt(120) != 0) {
            return false;
        }
        azw azw2 = vx.a(this.a, 10, 7);
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

