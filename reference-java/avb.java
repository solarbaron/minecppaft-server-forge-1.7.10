/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class avb
extends avc {
    public avb() {
    }

    public avb(int n2, Random random, asv asv2, int n3) {
        super(n2);
        this.g = n3;
        this.d = this.a(random);
        this.f = asv2;
    }

    @Override
    public void a(avk avk2, List list, Random random) {
        this.a((auz)avk2, list, random, 1, 1);
    }

    public static avb a(List list, Random random, int n2, int n3, int n4, int n5, int n6) {
        asv asv2 = asv.a(n2, n3, n4, -1, -7, 0, 5, 11, 8, n5);
        if (!avb.a(asv2) || avk.a(list, asv2) != null) {
            return null;
        }
        return new avb(n6, random, asv2, n5);
    }

    @Override
    public boolean a(ahb ahb2, Random random, asv asv2) {
        if (this.a(ahb2, asv2)) {
            return false;
        }
        this.a(ahb2, asv2, 0, 0, 0, 4, 10, 7, true, random, aui.c());
        this.a(ahb2, random, asv2, this.d, 1, 7, 0);
        this.a(ahb2, random, asv2, avd.a, 1, 1, 7);
        int n2 = this.a(ajn.ar, 2);
        for (int i2 = 0; i2 < 6; ++i2) {
            this.a(ahb2, ajn.ar, n2, 1, 6 - i2, 1 + i2, asv2);
            this.a(ahb2, ajn.ar, n2, 2, 6 - i2, 1 + i2, asv2);
            this.a(ahb2, ajn.ar, n2, 3, 6 - i2, 1 + i2, asv2);
            if (i2 >= 5) continue;
            this.a(ahb2, ajn.aV, 0, 1, 5 - i2, 1 + i2, asv2);
            this.a(ahb2, ajn.aV, 0, 2, 5 - i2, 1 + i2, asv2);
            this.a(ahb2, ajn.aV, 0, 3, 5 - i2, 1 + i2, asv2);
        }
        return true;
    }
}

