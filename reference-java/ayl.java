/*
 * Decompiled with CFR 0.152.
 */
public abstract class ayl {
    public final String h;
    private boolean a;

    public ayl(String string) {
        this.h = string;
    }

    public abstract void a(dh var1);

    public abstract void b(dh var1);

    public void c() {
        this.a(true);
    }

    public void a(boolean bl2) {
        this.a = bl2;
    }

    public boolean d() {
        return this.a;
    }
}

