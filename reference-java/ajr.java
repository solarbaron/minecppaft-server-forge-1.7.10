/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class ajr
extends aji {
    protected ajr(awt awt2) {
        super(awt2);
        this.a(true);
        float f2 = 0.2f;
        this.a(0.5f - f2, 0.0f, 0.5f - f2, 0.5f + f2, f2 * 3.0f, 0.5f + f2);
        this.a(abt.c);
    }

    protected ajr() {
        this(awt.k);
    }

    @Override
    public boolean c(ahb ahb2, int n2, int n3, int n4) {
        return super.c(ahb2, n2, n3, n4) && this.a(ahb2.a(n2, n3 - 1, n4));
    }

    protected boolean a(aji aji2) {
        return aji2 == ajn.c || aji2 == ajn.d || aji2 == ajn.ak;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2) {
        super.a(ahb2, n2, n3, n4, aji2);
        this.e(ahb2, n2, n3, n4);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, Random random) {
        this.e(ahb2, n2, n3, n4);
    }

    protected void e(ahb ahb2, int n2, int n3, int n4) {
        if (!this.j(ahb2, n2, n3, n4)) {
            this.b(ahb2, n2, n3, n4, ahb2.e(n2, n3, n4), 0);
            ahb2.d(n2, n3, n4, ajr.e(0), 0, 2);
        }
    }

    @Override
    public boolean j(ahb ahb2, int n2, int n3, int n4) {
        return this.a(ahb2.a(n2, n3 - 1, n4));
    }

    @Override
    public azt a(ahb ahb2, int n2, int n3, int n4) {
        return null;
    }

    @Override
    public boolean c() {
        return false;
    }

    @Override
    public boolean d() {
        return false;
    }

    @Override
    public int b() {
        return 1;
    }
}

