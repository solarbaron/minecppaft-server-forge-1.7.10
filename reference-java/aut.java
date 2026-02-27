/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class aut
extends avc {
    public aut() {
    }

    public aut(int n2, Random random, asv asv2, int n3) {
        super(n2);
        this.g = n3;
        this.d = this.a(random);
        this.f = asv2;
    }

    @Override
    public void a(avk avk2, List list, Random random) {
        this.a((auz)avk2, list, random, 1, 1);
    }

    public static aut a(List list, Random random, int n2, int n3, int n4, int n5, int n6) {
        asv asv2 = asv.a(n2, n3, n4, -1, -1, 0, 9, 5, 11, n5);
        if (!aut.a(asv2) || avk.a(list, asv2) != null) {
            return null;
        }
        return new aut(n6, random, asv2, n5);
    }

    @Override
    public boolean a(ahb ahb2, Random random, asv asv2) {
        if (this.a(ahb2, asv2)) {
            return false;
        }
        this.a(ahb2, asv2, 0, 0, 0, 8, 4, 10, true, random, aui.c());
        this.a(ahb2, random, asv2, this.d, 1, 1, 0);
        this.a(ahb2, asv2, 1, 1, 10, 3, 3, 10, ajn.a, ajn.a, false);
        this.a(ahb2, asv2, 4, 1, 1, 4, 3, 1, false, random, aui.c());
        this.a(ahb2, asv2, 4, 1, 3, 4, 3, 3, false, random, aui.c());
        this.a(ahb2, asv2, 4, 1, 7, 4, 3, 7, false, random, aui.c());
        this.a(ahb2, asv2, 4, 1, 9, 4, 3, 9, false, random, aui.c());
        this.a(ahb2, asv2, 4, 1, 4, 4, 3, 6, ajn.aY, ajn.aY, false);
        this.a(ahb2, asv2, 5, 1, 5, 7, 3, 5, ajn.aY, ajn.aY, false);
        this.a(ahb2, ajn.aY, 0, 4, 3, 2, asv2);
        this.a(ahb2, ajn.aY, 0, 4, 3, 8, asv2);
        this.a(ahb2, ajn.av, this.a(ajn.av, 3), 4, 1, 2, asv2);
        this.a(ahb2, ajn.av, this.a(ajn.av, 3) + 8, 4, 2, 2, asv2);
        this.a(ahb2, ajn.av, this.a(ajn.av, 3), 4, 1, 8, asv2);
        this.a(ahb2, ajn.av, this.a(ajn.av, 3) + 8, 4, 2, 8, asv2);
        return true;
    }
}

