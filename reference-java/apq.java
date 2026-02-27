/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class apq
extends aji {
    public apq() {
        super(awt.H);
        this.a(i);
        this.c(0.5f);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, int n5, yz yz2) {
        int n6;
        aji aji2;
        if (yz2.bE.d && ((aji2 = ahb2.a(n2 - q.b[n6 = apq.b(n5)], n3 - q.c[n6], n4 - q.d[n6])) == ajn.J || aji2 == ajn.F)) {
            ahb2.f(n2 - q.b[n6], n3 - q.c[n6], n4 - q.d[n6]);
        }
        super.a(ahb2, n2, n3, n4, n5, yz2);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2, int n5) {
        super.a(ahb2, n2, n3, n4, aji2, n5);
        int n6 = q.a[apq.b(n5)];
        aji aji3 = ahb2.a(n2 += q.b[n6], n3 += q.c[n6], n4 += q.d[n6]);
        if ((aji3 == ajn.J || aji3 == ajn.F) && app.c(n5 = ahb2.e(n2, n3, n4))) {
            aji3.b(ahb2, n2, n3, n4, n5, 0);
            ahb2.f(n2, n3, n4);
        }
    }

    @Override
    public int b() {
        return 17;
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
    public boolean c(ahb ahb2, int n2, int n3, int n4) {
        return false;
    }

    @Override
    public boolean d(ahb ahb2, int n2, int n3, int n4, int n5) {
        return false;
    }

    @Override
    public int a(Random random) {
        return 0;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, azt azt2, List list, sa sa2) {
        int n5 = ahb2.e(n2, n3, n4);
        float f2 = 0.25f;
        float f3 = 0.375f;
        float f4 = 0.625f;
        float f5 = 0.25f;
        float f6 = 0.75f;
        switch (apq.b(n5)) {
            case 0: {
                this.a(0.0f, 0.0f, 0.0f, 1.0f, 0.25f, 1.0f);
                super.a(ahb2, n2, n3, n4, azt2, list, sa2);
                this.a(0.375f, 0.25f, 0.375f, 0.625f, 1.0f, 0.625f);
                super.a(ahb2, n2, n3, n4, azt2, list, sa2);
                break;
            }
            case 1: {
                this.a(0.0f, 0.75f, 0.0f, 1.0f, 1.0f, 1.0f);
                super.a(ahb2, n2, n3, n4, azt2, list, sa2);
                this.a(0.375f, 0.0f, 0.375f, 0.625f, 0.75f, 0.625f);
                super.a(ahb2, n2, n3, n4, azt2, list, sa2);
                break;
            }
            case 2: {
                this.a(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.25f);
                super.a(ahb2, n2, n3, n4, azt2, list, sa2);
                this.a(0.25f, 0.375f, 0.25f, 0.75f, 0.625f, 1.0f);
                super.a(ahb2, n2, n3, n4, azt2, list, sa2);
                break;
            }
            case 3: {
                this.a(0.0f, 0.0f, 0.75f, 1.0f, 1.0f, 1.0f);
                super.a(ahb2, n2, n3, n4, azt2, list, sa2);
                this.a(0.25f, 0.375f, 0.0f, 0.75f, 0.625f, 0.75f);
                super.a(ahb2, n2, n3, n4, azt2, list, sa2);
                break;
            }
            case 4: {
                this.a(0.0f, 0.0f, 0.0f, 0.25f, 1.0f, 1.0f);
                super.a(ahb2, n2, n3, n4, azt2, list, sa2);
                this.a(0.375f, 0.25f, 0.25f, 0.625f, 0.75f, 1.0f);
                super.a(ahb2, n2, n3, n4, azt2, list, sa2);
                break;
            }
            case 5: {
                this.a(0.75f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
                super.a(ahb2, n2, n3, n4, azt2, list, sa2);
                this.a(0.0f, 0.375f, 0.25f, 0.75f, 0.625f, 0.75f);
                super.a(ahb2, n2, n3, n4, azt2, list, sa2);
            }
        }
        this.a(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    }

    @Override
    public void a(ahl ahl2, int n2, int n3, int n4) {
        int n5 = ahl2.e(n2, n3, n4);
        float f2 = 0.25f;
        switch (apq.b(n5)) {
            case 0: {
                this.a(0.0f, 0.0f, 0.0f, 1.0f, 0.25f, 1.0f);
                break;
            }
            case 1: {
                this.a(0.0f, 0.75f, 0.0f, 1.0f, 1.0f, 1.0f);
                break;
            }
            case 2: {
                this.a(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.25f);
                break;
            }
            case 3: {
                this.a(0.0f, 0.0f, 0.75f, 1.0f, 1.0f, 1.0f);
                break;
            }
            case 4: {
                this.a(0.0f, 0.0f, 0.0f, 0.25f, 1.0f, 1.0f);
                break;
            }
            case 5: {
                this.a(0.75f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
            }
        }
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2) {
        int n5 = apq.b(ahb2.e(n2, n3, n4));
        aji aji3 = ahb2.a(n2 - q.b[n5], n3 - q.c[n5], n4 - q.d[n5]);
        if (aji3 != ajn.J && aji3 != ajn.F) {
            ahb2.f(n2, n3, n4);
        } else {
            aji3.a(ahb2, n2 - q.b[n5], n3 - q.c[n5], n4 - q.d[n5], aji2);
        }
    }

    public static int b(int n2) {
        return qh.a(n2 & 7, 0, q.b.length - 1);
    }
}

