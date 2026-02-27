/*
 * Decompiled with CFR 0.152.
 */
public abstract class ang
extends aji {
    protected ang(awt awt2) {
        super(awt2);
    }

    @Override
    public int b() {
        return 31;
    }

    @Override
    public int a(ahb ahb2, int n2, int n3, int n4, int n5, float f2, float f3, float f4, int n6) {
        int n7 = n6 & 3;
        int n8 = 0;
        switch (n5) {
            case 2: 
            case 3: {
                n8 = 8;
                break;
            }
            case 4: 
            case 5: {
                n8 = 4;
                break;
            }
            case 0: 
            case 1: {
                n8 = 0;
            }
        }
        return n7 | n8;
    }

    @Override
    public int a(int n2) {
        return n2 & 3;
    }

    public int k(int n2) {
        return n2 & 3;
    }

    @Override
    protected add j(int n2) {
        return new add(adb.a(this), 1, this.k(n2));
    }
}

