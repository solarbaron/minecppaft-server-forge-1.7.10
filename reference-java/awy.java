/*
 * Decompiled with CFR 0.152.
 */
public class awy
extends axn {
    private final axa c;

    public awy(long l2, axn axn2, axa axa2) {
        super(l2);
        this.a = axn2;
        this.c = axa2;
    }

    @Override
    public int[] a(int n2, int n3, int n4, int n5) {
        switch (this.c) {
            default: {
                return this.c(n2, n3, n4, n5);
            }
            case b: {
                return this.d(n2, n3, n4, n5);
            }
            case c: 
        }
        return this.e(n2, n3, n4, n5);
    }

    private int[] c(int n2, int n3, int n4, int n5) {
        int n6 = n2 - 1;
        int n7 = n3 - 1;
        int n8 = 1 + n4 + 1;
        int n9 = 1 + n5 + 1;
        int[] nArray = this.a.a(n6, n7, n8, n9);
        int[] nArray2 = axl.a(n4 * n5);
        for (int i2 = 0; i2 < n5; ++i2) {
            for (int i3 = 0; i3 < n4; ++i3) {
                this.a((long)(i3 + n2), (long)(i2 + n3));
                int n10 = nArray[i3 + 1 + (i2 + 1) * n8];
                if (n10 == 1) {
                    boolean bl2;
                    int n11 = nArray[i3 + 1 + (i2 + 1 - 1) * n8];
                    int n12 = nArray[i3 + 1 + 1 + (i2 + 1) * n8];
                    int n13 = nArray[i3 + 1 - 1 + (i2 + 1) * n8];
                    int n14 = nArray[i3 + 1 + (i2 + 1 + 1) * n8];
                    boolean bl3 = n11 == 3 || n12 == 3 || n13 == 3 || n14 == 3;
                    boolean bl4 = bl2 = n11 == 4 || n12 == 4 || n13 == 4 || n14 == 4;
                    if (bl3 || bl2) {
                        n10 = 2;
                    }
                }
                nArray2[i3 + i2 * n4] = n10;
            }
        }
        return nArray2;
    }

    private int[] d(int n2, int n3, int n4, int n5) {
        int n6 = n2 - 1;
        int n7 = n3 - 1;
        int n8 = 1 + n4 + 1;
        int n9 = 1 + n5 + 1;
        int[] nArray = this.a.a(n6, n7, n8, n9);
        int[] nArray2 = axl.a(n4 * n5);
        for (int i2 = 0; i2 < n5; ++i2) {
            for (int i3 = 0; i3 < n4; ++i3) {
                int n10 = nArray[i3 + 1 + (i2 + 1) * n8];
                if (n10 == 4) {
                    boolean bl2;
                    int n11 = nArray[i3 + 1 + (i2 + 1 - 1) * n8];
                    int n12 = nArray[i3 + 1 + 1 + (i2 + 1) * n8];
                    int n13 = nArray[i3 + 1 - 1 + (i2 + 1) * n8];
                    int n14 = nArray[i3 + 1 + (i2 + 1 + 1) * n8];
                    boolean bl3 = n11 == 2 || n12 == 2 || n13 == 2 || n14 == 2;
                    boolean bl4 = bl2 = n11 == 1 || n12 == 1 || n13 == 1 || n14 == 1;
                    if (bl2 || bl3) {
                        n10 = 3;
                    }
                }
                nArray2[i3 + i2 * n4] = n10;
            }
        }
        return nArray2;
    }

    private int[] e(int n2, int n3, int n4, int n5) {
        int[] nArray = this.a.a(n2, n3, n4, n5);
        int[] nArray2 = axl.a(n4 * n5);
        for (int i2 = 0; i2 < n5; ++i2) {
            for (int i3 = 0; i3 < n4; ++i3) {
                this.a((long)(i3 + n2), (long)(i2 + n3));
                int n6 = nArray[i3 + i2 * n4];
                if (n6 != 0 && this.a(13) == 0) {
                    n6 |= 1 + this.a(15) << 8 & 0xF00;
                }
                nArray2[i3 + i2 * n4] = n6;
            }
        }
        return nArray2;
    }
}

