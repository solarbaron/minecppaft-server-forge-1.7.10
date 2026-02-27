/*
 * Decompiled with CFR 0.152.
 */
public class axq
extends axn {
    public axq(long l2, axn axn2) {
        super(l2);
        this.a = axn2;
    }

    @Override
    public int[] a(int n2, int n3, int n4, int n5) {
        int[] nArray = this.a.a(n2 - 1, n3 - 1, n4 + 2, n5 + 2);
        int[] nArray2 = axl.a(n4 * n5);
        for (int i2 = 0; i2 < n5; ++i2) {
            for (int i3 = 0; i3 < n4; ++i3) {
                this.a((long)(i3 + n2), (long)(i2 + n3));
                int n6 = nArray[i3 + 1 + (i2 + 1) * (n4 + 2)];
                if (this.a(57) == 0) {
                    if (n6 == ahu.p.ay) {
                        nArray2[i3 + i2 * n4] = ahu.p.ay + 128;
                        continue;
                    }
                    nArray2[i3 + i2 * n4] = n6;
                    continue;
                }
                nArray2[i3 + i2 * n4] = n6;
            }
        }
        return nArray2;
    }
}

