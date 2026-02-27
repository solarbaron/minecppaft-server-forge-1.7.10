/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class ava
extends avc {
    private boolean a;
    private boolean b;

    public ava() {
    }

    public ava(int n2, Random random, asv asv2, int n3) {
        super(n2);
        this.g = n3;
        this.d = this.a(random);
        this.f = asv2;
        this.a = random.nextInt(2) == 0;
        this.b = random.nextInt(2) == 0;
    }

    @Override
    protected void a(dh dh2) {
        super.a(dh2);
        dh2.a("Left", this.a);
        dh2.a("Right", this.b);
    }

    @Override
    protected void b(dh dh2) {
        super.b(dh2);
        this.a = dh2.n("Left");
        this.b = dh2.n("Right");
    }

    @Override
    public void a(avk avk2, List list, Random random) {
        this.a((auz)avk2, list, random, 1, 1);
        if (this.a) {
            this.b((auz)avk2, list, random, 1, 2);
        }
        if (this.b) {
            this.c((auz)avk2, list, random, 1, 2);
        }
    }

    public static ava a(List list, Random random, int n2, int n3, int n4, int n5, int n6) {
        asv asv2 = asv.a(n2, n3, n4, -1, -1, 0, 5, 5, 7, n5);
        if (!ava.a(asv2) || avk.a(list, asv2) != null) {
            return null;
        }
        return new ava(n6, random, asv2, n5);
    }

    @Override
    public boolean a(ahb ahb2, Random random, asv asv2) {
        if (this.a(ahb2, asv2)) {
            return false;
        }
        this.a(ahb2, asv2, 0, 0, 0, 4, 4, 6, true, random, aui.c());
        this.a(ahb2, random, asv2, this.d, 1, 1, 0);
        this.a(ahb2, random, asv2, avd.a, 1, 1, 6);
        this.a(ahb2, asv2, random, 0.1f, 1, 2, 1, ajn.aa, 0);
        this.a(ahb2, asv2, random, 0.1f, 3, 2, 1, ajn.aa, 0);
        this.a(ahb2, asv2, random, 0.1f, 1, 2, 5, ajn.aa, 0);
        this.a(ahb2, asv2, random, 0.1f, 3, 2, 5, ajn.aa, 0);
        if (this.a) {
            this.a(ahb2, asv2, 0, 1, 2, 0, 3, 4, ajn.a, ajn.a, false);
        }
        if (this.b) {
            this.a(ahb2, asv2, 4, 1, 2, 4, 3, 4, ajn.a, ajn.a, false);
        }
        return true;
    }
}

