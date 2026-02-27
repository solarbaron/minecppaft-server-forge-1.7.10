/*
 * Decompiled with CFR 0.152.
 */
public class acx
extends adb {
    public final int a = 32;
    private final int b;
    private final float c;
    private final boolean d;
    private boolean m;
    private int n;
    private int o;
    private int p;
    private float q;

    public acx(int n2, float f2, boolean bl2) {
        this.b = n2;
        this.d = bl2;
        this.c = f2;
        this.a(abt.h);
    }

    public acx(int n2, boolean bl2) {
        this(n2, 0.6f, bl2);
    }

    @Override
    public add b(add add2, ahb ahb2, yz yz2) {
        --add2.b;
        yz2.bQ().a(this, add2);
        ahb2.a((sa)yz2, "random.burp", 0.5f, ahb2.s.nextFloat() * 0.1f + 0.9f);
        this.c(add2, ahb2, yz2);
        return add2;
    }

    protected void c(add add2, ahb ahb2, yz yz2) {
        if (!ahb2.E && this.n > 0 && ahb2.s.nextFloat() < this.q) {
            yz2.c(new rw(this.n, this.o * 20, this.p));
        }
    }

    @Override
    public int d_(add add2) {
        return 32;
    }

    @Override
    public aei d(add add2) {
        return aei.b;
    }

    @Override
    public add a(add add2, ahb ahb2, yz yz2) {
        if (yz2.g(this.m)) {
            yz2.a(add2, this.d_(add2));
        }
        return add2;
    }

    public int g(add add2) {
        return this.b;
    }

    public float h(add add2) {
        return this.c;
    }

    public boolean i() {
        return this.d;
    }

    public acx a(int n2, int n3, int n4, float f2) {
        this.n = n2;
        this.o = n3;
        this.p = n4;
        this.q = f2;
        return this;
    }

    public acx j() {
        this.m = true;
        return this;
    }
}

