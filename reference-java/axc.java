/*
 * Decompiled with CFR 0.152.
 */
public class axc
extends axn {
    public axc(long l2, axn axn2) {
        super(l2);
        this.a = axn2;
    }

    @Override
    public int[] a(int n2, int n3, int n4, int n5) {
        int n6 = n2 - 1;
        int n7 = n3 - 1;
        int n8 = n4 + 2;
        int n9 = n5 + 2;
        int[] nArray = this.a.a(n6, n7, n8, n9);
        int[] nArray2 = axl.a(n4 * n5);
        for (int i2 = 0; i2 < n5; ++i2) {
            for (int i3 = 0; i3 < n4; ++i3) {
                int n10 = nArray[i3 + 0 + (i2 + 0) * n8];
                int n11 = nArray[i3 + 2 + (i2 + 0) * n8];
                int n12 = nArray[i3 + 0 + (i2 + 2) * n8];
                int n13 = nArray[i3 + 2 + (i2 + 2) * n8];
                int n14 = nArray[i3 + 1 + (i2 + 1) * n8];
                this.a((long)(i3 + n2), (long)(i2 + n3));
                nArray2[i3 + i2 * n4] = n14 == 0 && n10 == 0 && n11 == 0 && n12 == 0 && n13 == 0 && this.a(100) == 0 ? ahu.C.ay : n14;
            }
        }
        return nArray2;
    }
}

