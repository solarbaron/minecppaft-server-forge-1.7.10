/*
 * Decompiled with CFR 0.152.
 */
public class amx
extends aji {
    public static final String[] a = new String[]{"default", "chiseled", "lines"};
    private static final String[] b = new String[]{"side", "chiseled", "lines", null, null};

    public amx() {
        super(awt.e);
        this.a(abt.b);
    }

    @Override
    public int a(ahb ahb2, int n2, int n3, int n4, int n5, float f2, float f3, float f4, int n6) {
        if (n6 == 2) {
            switch (n5) {
                case 2: 
                case 3: {
                    n6 = 4;
                    break;
                }
                case 4: 
                case 5: {
                    n6 = 3;
                    break;
                }
                case 0: 
                case 1: {
                    n6 = 2;
                }
            }
        }
        return n6;
    }

    @Override
    public int a(int n2) {
        if (n2 == 3 || n2 == 4) {
            return 2;
        }
        return n2;
    }

    @Override
    protected add j(int n2) {
        if (n2 == 3 || n2 == 4) {
            return new add(adb.a(this), 1, 2);
        }
        return super.j(n2);
    }

    @Override
    public int b() {
        return 39;
    }

    @Override
    public awv f(int n2) {
        return awv.p;
    }
}

