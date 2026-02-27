/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;
import org.apache.commons.lang3.tuple.ImmutablePair;

public class amb
extends aji {
    public static final String[] a = new String[]{"stone", "cobble", "brick", "mossybrick", "crackedbrick", "chiseledbrick"};

    public amb() {
        super(awt.B);
        this.c(0.0f);
        this.a(abt.c);
    }

    @Override
    public void b(ahb ahb2, int n2, int n3, int n4, int n5) {
        if (!ahb2.E) {
            yk yk2 = new yk(ahb2);
            yk2.b((double)n2 + 0.5, n3, (double)n4 + 0.5, 0.0f, 0.0f);
            ahb2.d(yk2);
            yk2.s();
        }
        super.b(ahb2, n2, n3, n4, n5);
    }

    @Override
    public int a(Random random) {
        return 0;
    }

    public static boolean a(aji aji2) {
        return aji2 == ajn.b || aji2 == ajn.e || aji2 == ajn.aV;
    }

    public static int a(aji aji2, int n2) {
        if (n2 == 0) {
            if (aji2 == ajn.e) {
                return 1;
            }
            if (aji2 == ajn.aV) {
                return 2;
            }
        } else if (aji2 == ajn.aV) {
            switch (n2) {
                case 1: {
                    return 3;
                }
                case 2: {
                    return 4;
                }
                case 3: {
                    return 5;
                }
            }
        }
        return 0;
    }

    public static ImmutablePair b(int n2) {
        switch (n2) {
            case 1: {
                return new ImmutablePair<aji, Integer>(ajn.e, 0);
            }
            case 2: {
                return new ImmutablePair<aji, Integer>(ajn.aV, 0);
            }
            case 3: {
                return new ImmutablePair<aji, Integer>(ajn.aV, 1);
            }
            case 4: {
                return new ImmutablePair<aji, Integer>(ajn.aV, 2);
            }
            case 5: {
                return new ImmutablePair<aji, Integer>(ajn.aV, 3);
            }
        }
        return new ImmutablePair<aji, Integer>(ajn.b, 0);
    }

    @Override
    protected add j(int n2) {
        switch (n2) {
            case 1: {
                return new add(ajn.e);
            }
            case 2: {
                return new add(ajn.aV);
            }
            case 3: {
                return new add(ajn.aV, 1, 1);
            }
            case 4: {
                return new add(ajn.aV, 1, 2);
            }
            case 5: {
                return new add(ajn.aV, 1, 3);
            }
        }
        return new add(ajn.b);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, int n5, float f2, int n6) {
        if (!ahb2.E) {
            yk yk2 = new yk(ahb2);
            yk2.b((double)n2 + 0.5, n3, (double)n4 + 0.5, 0.0f, 0.0f);
            ahb2.d(yk2);
            yk2.s();
        }
    }

    @Override
    public int k(ahb ahb2, int n2, int n3, int n4) {
        return ahb2.e(n2, n3, n4);
    }
}

