/*
 * Decompiled with CFR 0.152.
 */
class kw
extends kk {
    final /* synthetic */ add b;
    final /* synthetic */ kv c;

    kw(kv kv2, add add2) {
        this.c = kv2;
        this.b = add2;
    }

    @Override
    protected zh a(ahb ahb2, cx cx2) {
        return new zo(ahb2, cx2.a(), cx2.b(), cx2.c(), this.b.m());
    }

    @Override
    protected float a() {
        return super.a() * 0.5f;
    }

    @Override
    protected float b() {
        return super.b() * 1.25f;
    }
}

