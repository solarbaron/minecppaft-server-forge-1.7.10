/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class ath
extends ats {
    private int b;

    public ath() {
    }

    public ath(int n2, Random random, asv asv2, int n3) {
        super(n2);
        this.g = n3;
        this.f = asv2;
        this.b = random.nextInt();
    }

    public static ath a(List list, Random random, int n2, int n3, int n4, int n5, int n6) {
        asv asv2 = asv.a(n2, n3, n4, -1, -3, 0, 5, 10, 8, n5);
        if (!ath.a(asv2) || avk.a(list, asv2) != null) {
            return null;
        }
        return new ath(n6, random, asv2, n5);
    }

    @Override
    protected void b(dh dh2) {
        super.b(dh2);
        this.b = dh2.f("Seed");
    }

    @Override
    protected void a(dh dh2) {
        super.a(dh2);
        dh2.a("Seed", this.b);
    }

    @Override
    public boolean a(ahb ahb2, Random random, asv asv2) {
        int n2;
        int n3;
        int n4;
        Random random2 = new Random(this.b);
        for (n4 = 0; n4 <= 4; ++n4) {
            for (n3 = 3; n3 <= 4; ++n3) {
                n2 = random2.nextInt(8);
                this.a(ahb2, asv2, n4, n3, 0, n4, n3, n2, ajn.bj, ajn.bj, false);
            }
        }
        n4 = random2.nextInt(8);
        this.a(ahb2, asv2, 0, 5, 0, 0, 5, n4, ajn.bj, ajn.bj, false);
        n4 = random2.nextInt(8);
        this.a(ahb2, asv2, 4, 5, 0, 4, 5, n4, ajn.bj, ajn.bj, false);
        for (n4 = 0; n4 <= 4; ++n4) {
            n3 = random2.nextInt(5);
            this.a(ahb2, asv2, n4, 2, 0, n4, 2, n3, ajn.bj, ajn.bj, false);
        }
        for (n4 = 0; n4 <= 4; ++n4) {
            for (n3 = 0; n3 <= 1; ++n3) {
                n2 = random2.nextInt(3);
                this.a(ahb2, asv2, n4, n3, 0, n4, n3, n2, ajn.bj, ajn.bj, false);
            }
        }
        return true;
    }
}

