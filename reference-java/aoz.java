/*
 * Decompiled with CFR 0.152.
 */
public class aoz
extends aor {
    private int a;

    @Override
    public void b(dh dh2) {
        super.b(dh2);
        dh2.a("OutputSignal", this.a);
    }

    @Override
    public void a(dh dh2) {
        super.a(dh2);
        this.a = dh2.f("OutputSignal");
    }

    public int a() {
        return this.a;
    }

    public void a(int n2) {
        this.a = n2;
    }
}

