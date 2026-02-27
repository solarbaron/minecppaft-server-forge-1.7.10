/*
 * Decompiled with CFR 0.152.
 */
public class ahr
implements ahl {
    private int a;
    private int b;
    private apx[][] c;
    private boolean d;
    private ahb e;

    public ahr(ahb ahb2, int n2, int n3, int n4, int n5, int n6, int n7, int n8) {
        apx apx2;
        int n9;
        int n10;
        this.e = ahb2;
        this.a = n2 - n8 >> 4;
        this.b = n4 - n8 >> 4;
        int n11 = n5 + n8 >> 4;
        int n12 = n7 + n8 >> 4;
        this.c = new apx[n11 - this.a + 1][n12 - this.b + 1];
        this.d = true;
        for (n10 = this.a; n10 <= n11; ++n10) {
            for (n9 = this.b; n9 <= n12; ++n9) {
                apx2 = ahb2.e(n10, n9);
                if (apx2 == null) continue;
                this.c[n10 - this.a][n9 - this.b] = apx2;
            }
        }
        for (n10 = n2 >> 4; n10 <= n5 >> 4; ++n10) {
            for (n9 = n4 >> 4; n9 <= n7 >> 4; ++n9) {
                apx2 = this.c[n10 - this.a][n9 - this.b];
                if (apx2 == null || apx2.c(n3, n6)) continue;
                this.d = false;
            }
        }
    }

    @Override
    public aji a(int n2, int n3, int n4) {
        aji aji2 = ajn.a;
        if (n3 >= 0 && n3 < 256) {
            apx apx2;
            int n5 = (n2 >> 4) - this.a;
            int n6 = (n4 >> 4) - this.b;
            if (n5 >= 0 && n5 < this.c.length && n6 >= 0 && n6 < this.c[n5].length && (apx2 = this.c[n5][n6]) != null) {
                aji2 = apx2.a(n2 & 0xF, n3, n4 & 0xF);
            }
        }
        return aji2;
    }

    @Override
    public aor o(int n2, int n3, int n4) {
        int n5 = (n2 >> 4) - this.a;
        int n6 = (n4 >> 4) - this.b;
        return this.c[n5][n6].e(n2 & 0xF, n3, n4 & 0xF);
    }

    @Override
    public int e(int n2, int n3, int n4) {
        if (n3 < 0) {
            return 0;
        }
        if (n3 >= 256) {
            return 0;
        }
        int n5 = (n2 >> 4) - this.a;
        int n6 = (n4 >> 4) - this.b;
        return this.c[n5][n6].c(n2 & 0xF, n3, n4 & 0xF);
    }

    @Override
    public int e(int n2, int n3, int n4, int n5) {
        return this.a(n2, n3, n4).c(this, n2, n3, n4, n5);
    }
}

