/*
 * Decompiled with CFR 0.152.
 */
public class apj
extends aor {
    private final agq a = new apk(this);

    @Override
    public void a(dh dh2) {
        super.a(dh2);
        this.a.a(dh2);
    }

    @Override
    public void b(dh dh2) {
        super.b(dh2);
        this.a.b(dh2);
    }

    @Override
    public void h() {
        this.a.g();
        super.h();
    }

    @Override
    public ft m() {
        dh dh2 = new dh();
        this.b(dh2);
        dh2.o("SpawnPotentials");
        return new gf(this.c, this.d, this.e, 1, dh2);
    }

    @Override
    public boolean c(int n2, int n3) {
        if (this.a.b(n2)) {
            return true;
        }
        return super.c(n2, n3);
    }

    public agq a() {
        return this.a;
    }
}

