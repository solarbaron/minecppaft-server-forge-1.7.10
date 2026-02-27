/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class auu
extends aup {
    @Override
    public void a(avk avk2, List list, Random random) {
        if (this.g == 2 || this.g == 3) {
            this.c((auz)avk2, list, random, 1, 1);
        } else {
            this.b((auz)avk2, list, random, 1, 1);
        }
    }

    @Override
    public boolean a(ahb ahb2, Random random, asv asv2) {
        if (this.a(ahb2, asv2)) {
            return false;
        }
        this.a(ahb2, asv2, 0, 0, 0, 4, 4, 4, true, random, aui.c());
        this.a(ahb2, random, asv2, this.d, 1, 1, 0);
        if (this.g == 2 || this.g == 3) {
            this.a(ahb2, asv2, 4, 1, 1, 4, 3, 3, ajn.a, ajn.a, false);
        } else {
            this.a(ahb2, asv2, 0, 1, 1, 0, 3, 3, ajn.a, ajn.a, false);
        }
        return true;
    }
}

