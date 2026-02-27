/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class aup
extends avc {
    public aup() {
    }

    public aup(int n2, Random random, asv asv2, int n3) {
        super(n2);
        this.g = n3;
        this.d = this.a(random);
        this.f = asv2;
    }

    @Override
    public void a(avk avk2, List list, Random random) {
        if (this.g == 2 || this.g == 3) {
            this.b((auz)avk2, list, random, 1, 1);
        } else {
            this.c((auz)avk2, list, random, 1, 1);
        }
    }

    public static aup a(List list, Random random, int n2, int n3, int n4, int n5, int n6) {
        asv asv2 = asv.a(n2, n3, n4, -1, -1, 0, 5, 5, 5, n5);
        if (!aup.a(asv2) || avk.a(list, asv2) != null) {
            return null;
        }
        return new aup(n6, random, asv2, n5);
    }

    @Override
    public boolean a(ahb ahb2, Random random, asv asv2) {
        if (this.a(ahb2, asv2)) {
            return false;
        }
        this.a(ahb2, asv2, 0, 0, 0, 4, 4, 4, true, random, aui.c());
        this.a(ahb2, random, asv2, this.d, 1, 1, 0);
        if (this.g == 2 || this.g == 3) {
            this.a(ahb2, asv2, 0, 1, 1, 0, 3, 3, ajn.a, ajn.a, false);
        } else {
            this.a(ahb2, asv2, 4, 1, 1, 4, 3, 3, ajn.a, ajn.a, false);
        }
        return true;
    }
}

