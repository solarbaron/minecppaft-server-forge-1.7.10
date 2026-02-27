/*
 * Decompiled with CFR 0.152.
 */
public class apf
extends aor {
    private adb a;
    private int i;

    public apf() {
    }

    public apf(adb adb2, int n2) {
        this.a = adb2;
        this.i = n2;
    }

    @Override
    public void b(dh dh2) {
        super.b(dh2);
        dh2.a("Item", adb.b(this.a));
        dh2.a("Data", this.i);
    }

    @Override
    public void a(dh dh2) {
        super.a(dh2);
        this.a = adb.d(dh2.f("Item"));
        this.i = dh2.f("Data");
    }

    @Override
    public ft m() {
        dh dh2 = new dh();
        this.b(dh2);
        return new gf(this.c, this.d, this.e, 5, dh2);
    }

    public void a(adb adb2, int n2) {
        this.a = adb2;
        this.i = n2;
    }

    public adb a() {
        return this.a;
    }

    public int b() {
        return this.i;
    }
}

