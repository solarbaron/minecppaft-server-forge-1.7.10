/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

abstract class avc
extends avk {
    protected avd d = avd.a;

    public avc() {
    }

    protected avc(int n2) {
        super(n2);
    }

    @Override
    protected void a(dh dh2) {
        dh2.a("EntryDoor", this.d.name());
    }

    @Override
    protected void b(dh dh2) {
        this.d = avd.valueOf(dh2.j("EntryDoor"));
    }

    protected void a(ahb ahb2, Random random, asv asv2, avd avd2, int n2, int n3, int n4) {
        switch (avd2) {
            default: {
                this.a(ahb2, asv2, n2, n3, n4, n2 + 3 - 1, n3 + 3 - 1, n4, ajn.a, ajn.a, false);
                break;
            }
            case b: {
                this.a(ahb2, ajn.aV, 0, n2, n3, n4, asv2);
                this.a(ahb2, ajn.aV, 0, n2, n3 + 1, n4, asv2);
                this.a(ahb2, ajn.aV, 0, n2, n3 + 2, n4, asv2);
                this.a(ahb2, ajn.aV, 0, n2 + 1, n3 + 2, n4, asv2);
                this.a(ahb2, ajn.aV, 0, n2 + 2, n3 + 2, n4, asv2);
                this.a(ahb2, ajn.aV, 0, n2 + 2, n3 + 1, n4, asv2);
                this.a(ahb2, ajn.aV, 0, n2 + 2, n3, n4, asv2);
                this.a(ahb2, ajn.ao, 0, n2 + 1, n3, n4, asv2);
                this.a(ahb2, ajn.ao, 8, n2 + 1, n3 + 1, n4, asv2);
                break;
            }
            case c: {
                this.a(ahb2, ajn.a, 0, n2 + 1, n3, n4, asv2);
                this.a(ahb2, ajn.a, 0, n2 + 1, n3 + 1, n4, asv2);
                this.a(ahb2, ajn.aY, 0, n2, n3, n4, asv2);
                this.a(ahb2, ajn.aY, 0, n2, n3 + 1, n4, asv2);
                this.a(ahb2, ajn.aY, 0, n2, n3 + 2, n4, asv2);
                this.a(ahb2, ajn.aY, 0, n2 + 1, n3 + 2, n4, asv2);
                this.a(ahb2, ajn.aY, 0, n2 + 2, n3 + 2, n4, asv2);
                this.a(ahb2, ajn.aY, 0, n2 + 2, n3 + 1, n4, asv2);
                this.a(ahb2, ajn.aY, 0, n2 + 2, n3, n4, asv2);
                break;
            }
            case d: {
                this.a(ahb2, ajn.aV, 0, n2, n3, n4, asv2);
                this.a(ahb2, ajn.aV, 0, n2, n3 + 1, n4, asv2);
                this.a(ahb2, ajn.aV, 0, n2, n3 + 2, n4, asv2);
                this.a(ahb2, ajn.aV, 0, n2 + 1, n3 + 2, n4, asv2);
                this.a(ahb2, ajn.aV, 0, n2 + 2, n3 + 2, n4, asv2);
                this.a(ahb2, ajn.aV, 0, n2 + 2, n3 + 1, n4, asv2);
                this.a(ahb2, ajn.aV, 0, n2 + 2, n3, n4, asv2);
                this.a(ahb2, ajn.av, 0, n2 + 1, n3, n4, asv2);
                this.a(ahb2, ajn.av, 8, n2 + 1, n3 + 1, n4, asv2);
                this.a(ahb2, ajn.aB, this.a(ajn.aB, 4), n2 + 2, n3 + 1, n4 + 1, asv2);
                this.a(ahb2, ajn.aB, this.a(ajn.aB, 3), n2 + 2, n3 + 1, n4 - 1, asv2);
            }
        }
    }

    protected avd a(Random random) {
        int n2 = random.nextInt(5);
        switch (n2) {
            default: {
                return avd.a;
            }
            case 2: {
                return avd.b;
            }
            case 3: {
                return avd.c;
            }
            case 4: 
        }
        return avd.d;
    }

    protected avk a(auz auz2, List list, Random random, int n2, int n3) {
        switch (this.g) {
            case 2: {
                return aui.a(auz2, list, random, this.f.a + n2, this.f.b + n3, this.f.c - 1, this.g, this.d());
            }
            case 0: {
                return aui.a(auz2, list, random, this.f.a + n2, this.f.b + n3, this.f.f + 1, this.g, this.d());
            }
            case 1: {
                return aui.a(auz2, list, random, this.f.a - 1, this.f.b + n3, this.f.c + n2, this.g, this.d());
            }
            case 3: {
                return aui.a(auz2, list, random, this.f.d + 1, this.f.b + n3, this.f.c + n2, this.g, this.d());
            }
        }
        return null;
    }

    protected avk b(auz auz2, List list, Random random, int n2, int n3) {
        switch (this.g) {
            case 2: {
                return aui.a(auz2, list, random, this.f.a - 1, this.f.b + n2, this.f.c + n3, 1, this.d());
            }
            case 0: {
                return aui.a(auz2, list, random, this.f.a - 1, this.f.b + n2, this.f.c + n3, 1, this.d());
            }
            case 1: {
                return aui.a(auz2, list, random, this.f.a + n3, this.f.b + n2, this.f.c - 1, 2, this.d());
            }
            case 3: {
                return aui.a(auz2, list, random, this.f.a + n3, this.f.b + n2, this.f.c - 1, 2, this.d());
            }
        }
        return null;
    }

    protected avk c(auz auz2, List list, Random random, int n2, int n3) {
        switch (this.g) {
            case 2: {
                return aui.a(auz2, list, random, this.f.d + 1, this.f.b + n2, this.f.c + n3, 3, this.d());
            }
            case 0: {
                return aui.a(auz2, list, random, this.f.d + 1, this.f.b + n2, this.f.c + n3, 3, this.d());
            }
            case 1: {
                return aui.a(auz2, list, random, this.f.a + n3, this.f.b + n2, this.f.f + 1, 0, this.d());
            }
            case 3: {
                return aui.a(auz2, list, random, this.f.a + n3, this.f.b + n2, this.f.f + 1, 0, this.d());
            }
        }
        return null;
    }

    protected static boolean a(asv asv2) {
        return asv2 != null && asv2.b > 10;
    }
}

