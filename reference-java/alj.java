/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public abstract class alj
extends aji {
    protected final boolean a;

    public alj(boolean bl2, awt awt2) {
        super(awt2);
        this.a = bl2;
        if (bl2) {
            this.q = true;
        } else {
            this.a(0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f);
        }
        this.g(255);
    }

    @Override
    public void a(ahl ahl2, int n2, int n3, int n4) {
        if (this.a) {
            this.a(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
        } else {
            boolean bl2;
            boolean bl3 = bl2 = (ahl2.e(n2, n3, n4) & 8) != 0;
            if (bl2) {
                this.a(0.0f, 0.5f, 0.0f, 1.0f, 1.0f, 1.0f);
            } else {
                this.a(0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f);
            }
        }
    }

    @Override
    public void g() {
        if (this.a) {
            this.a(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
        } else {
            this.a(0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f);
        }
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, azt azt2, List list, sa sa2) {
        this.a((ahl)ahb2, n2, n3, n4);
        super.a(ahb2, n2, n3, n4, azt2, list, sa2);
    }

    @Override
    public boolean c() {
        return this.a;
    }

    @Override
    public int a(ahb ahb2, int n2, int n3, int n4, int n5, float f2, float f3, float f4, int n6) {
        if (this.a) {
            return n6;
        }
        if (n5 == 0 || n5 != 1 && (double)f3 > 0.5) {
            return n6 | 8;
        }
        return n6;
    }

    @Override
    public int a(Random random) {
        if (this.a) {
            return 2;
        }
        return 1;
    }

    @Override
    public int a(int n2) {
        return n2 & 7;
    }

    @Override
    public boolean d() {
        return this.a;
    }

    public abstract String b(int var1);

    @Override
    public int k(ahb ahb2, int n2, int n3, int n4) {
        return super.k(ahb2, n2, n3, n4) & 7;
    }
}

