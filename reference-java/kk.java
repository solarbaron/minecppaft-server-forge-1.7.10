/*
 * Decompiled with CFR 0.152.
 */
public abstract class kk
extends cm {
    @Override
    public add b(ck ck2, add add2) {
        ahb ahb2 = ck2.k();
        cx cx2 = akm.a(ck2);
        cr cr2 = akm.b(ck2.h());
        zh zh2 = this.a(ahb2, cx2);
        zh2.c(cr2.c(), (float)cr2.d() + 0.1f, cr2.e(), this.b(), this.a());
        ahb2.d((sa)((Object)zh2));
        add2.a(1);
        return add2;
    }

    @Override
    protected void a(ck ck2) {
        ck2.k().c(1002, ck2.d(), ck2.e(), ck2.f(), 0);
    }

    protected abstract zh a(ahb var1, cx var2);

    protected float a() {
        return 6.0f;
    }

    protected float b() {
        return 1.1f;
    }
}

