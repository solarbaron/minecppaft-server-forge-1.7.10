/*
 * Decompiled with CFR 0.152.
 */
public class act
extends acx {
    private final boolean b;

    public act(boolean bl2) {
        super(0, 0.0f, false);
        this.b = bl2;
    }

    @Override
    public int g(add add2) {
        acu acu2 = acu.a(add2);
        if (this.b && acu2.i()) {
            return acu2.e();
        }
        return acu2.c();
    }

    @Override
    public float h(add add2) {
        acu acu2 = acu.a(add2);
        if (this.b && acu2.i()) {
            return acu2.f();
        }
        return acu2.d();
    }

    @Override
    public String i(add add2) {
        if (acu.a(add2) == acu.d) {
            return aen.m;
        }
        return null;
    }

    @Override
    protected void c(add add2, ahb ahb2, yz yz2) {
        acu acu2 = acu.a(add2);
        if (acu2 == acu.d) {
            yz2.c(new rw(rv.u.H, 1200, 3));
            yz2.c(new rw(rv.s.H, 300, 2));
            yz2.c(new rw(rv.k.H, 300, 1));
        }
        super.c(add2, ahb2, yz2);
    }

    @Override
    public String a(add add2) {
        acu acu2 = acu.a(add2);
        return this.a() + "." + acu2.b() + "." + (this.b && acu2.i() ? "cooked" : "raw");
    }
}

