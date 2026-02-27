/*
 * Decompiled with CFR 0.152.
 */
public class axt
extends axn {
    public axt(long l2, axn axn2) {
        super(l2);
        this.a = axn2;
    }

    @Override
    public int[] a(int n2, int n3, int n4, int n5) {
        int[] nArray = this.a.a(n2, n3, n4, n5);
        int[] nArray2 = axl.a(n4 * n5);
        for (int i2 = 0; i2 < n5; ++i2) {
            for (int i3 = 0; i3 < n4; ++i3) {
                this.a((long)(i3 + n2), (long)(i2 + n3));
                nArray2[i3 + i2 * n4] = nArray[i3 + i2 * n4] > 0 ? this.a(299999) + 2 : 0;
            }
        }
        return nArray2;
    }
}

