/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class ant
extends alw {
    protected ant(awt awt2) {
        super(awt2);
        this.a(false);
        if (awt2 == awt.i) {
            this.a(true);
        }
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2) {
        super.a(ahb2, n2, n3, n4, aji2);
        if (ahb2.a(n2, n3, n4) == this) {
            this.n(ahb2, n2, n3, n4);
        }
    }

    private void n(ahb ahb2, int n2, int n3, int n4) {
        int n5 = ahb2.e(n2, n3, n4);
        ahb2.d(n2, n3, n4, aji.e(aji.b(this) - 1), n5, 2);
        ahb2.a(n2, n3, n4, aji.e(aji.b(this) - 1), this.a(ahb2));
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, Random random) {
        if (this.J == awt.i) {
            int n5;
            int n6 = random.nextInt(3);
            for (n5 = 0; n5 < n6; ++n5) {
                aji aji2 = ahb2.a(n2 += random.nextInt(3) - 1, ++n3, n4 += random.nextInt(3) - 1);
                if (aji2.J == awt.a) {
                    if (!this.o(ahb2, n2 - 1, n3, n4) && !this.o(ahb2, n2 + 1, n3, n4) && !this.o(ahb2, n2, n3, n4 - 1) && !this.o(ahb2, n2, n3, n4 + 1) && !this.o(ahb2, n2, n3 - 1, n4) && !this.o(ahb2, n2, n3 + 1, n4)) continue;
                    ahb2.b(n2, n3, n4, ajn.ab);
                    return;
                }
                if (!aji2.J.c()) continue;
                return;
            }
            if (n6 == 0) {
                n5 = n2;
                int n7 = n4;
                for (int i2 = 0; i2 < 3; ++i2) {
                    n2 = n5 + random.nextInt(3) - 1;
                    if (!ahb2.c(n2, n3 + 1, n4 = n7 + random.nextInt(3) - 1) || !this.o(ahb2, n2, n3, n4)) continue;
                    ahb2.b(n2, n3 + 1, n4, ajn.ab);
                }
            }
        }
    }

    private boolean o(ahb ahb2, int n2, int n3, int n4) {
        return ahb2.a(n2, n3, n4).o().h();
    }
}

