/*
 * Decompiled with CFR 0.152.
 */
public class ayb
extends axn {
    public ayb(long l2, axn axn2) {
        super(l2);
        this.a = axn2;
    }

    @Override
    public int[] a(int n2, int n3, int n4, int n5) {
        int n6;
        int n7 = (n2 -= 2) >> 2;
        int n8 = (n3 -= 2) >> 2;
        int n9 = (n4 >> 2) + 2;
        int n10 = (n5 >> 2) + 2;
        int[] nArray = this.a.a(n7, n8, n9, n10);
        int n11 = n9 - 1 << 2;
        int n12 = n10 - 1 << 2;
        int[] nArray2 = axl.a(n11 * n12);
        for (int i2 = 0; i2 < n10 - 1; ++i2) {
            int n13 = nArray[n6 + 0 + (i2 + 0) * n9];
            int n14 = nArray[n6 + 0 + (i2 + 1) * n9];
            for (n6 = 0; n6 < n9 - 1; ++n6) {
                double d2 = 3.6;
                this.a((long)(n6 + n7 << 2), (long)(i2 + n8 << 2));
                double d3 = ((double)this.a(1024) / 1024.0 - 0.5) * 3.6;
                double d4 = ((double)this.a(1024) / 1024.0 - 0.5) * 3.6;
                this.a((long)(n6 + n7 + 1 << 2), (long)(i2 + n8 << 2));
                double d5 = ((double)this.a(1024) / 1024.0 - 0.5) * 3.6 + 4.0;
                double d6 = ((double)this.a(1024) / 1024.0 - 0.5) * 3.6;
                this.a((long)(n6 + n7 << 2), (long)(i2 + n8 + 1 << 2));
                double d7 = ((double)this.a(1024) / 1024.0 - 0.5) * 3.6;
                double d8 = ((double)this.a(1024) / 1024.0 - 0.5) * 3.6 + 4.0;
                this.a((long)(n6 + n7 + 1 << 2), (long)(i2 + n8 + 1 << 2));
                double d9 = ((double)this.a(1024) / 1024.0 - 0.5) * 3.6 + 4.0;
                double d10 = ((double)this.a(1024) / 1024.0 - 0.5) * 3.6 + 4.0;
                int n15 = nArray[n6 + 1 + (i2 + 0) * n9] & 0xFF;
                int n16 = nArray[n6 + 1 + (i2 + 1) * n9] & 0xFF;
                for (int i3 = 0; i3 < 4; ++i3) {
                    int n17 = ((i2 << 2) + i3) * n11 + (n6 << 2);
                    for (int i4 = 0; i4 < 4; ++i4) {
                        double d11 = ((double)i3 - d4) * ((double)i3 - d4) + ((double)i4 - d3) * ((double)i4 - d3);
                        double d12 = ((double)i3 - d6) * ((double)i3 - d6) + ((double)i4 - d5) * ((double)i4 - d5);
                        double d13 = ((double)i3 - d8) * ((double)i3 - d8) + ((double)i4 - d7) * ((double)i4 - d7);
                        double d14 = ((double)i3 - d10) * ((double)i3 - d10) + ((double)i4 - d9) * ((double)i4 - d9);
                        nArray2[n17++] = d11 < d12 && d11 < d13 && d11 < d14 ? n13 : (d12 < d11 && d12 < d13 && d12 < d14 ? n15 : (d13 < d11 && d13 < d12 && d13 < d14 ? n14 : n16));
                    }
                }
                n13 = n15;
                n14 = n16;
            }
        }
        int[] nArray3 = axl.a(n4 * n5);
        for (n6 = 0; n6 < n5; ++n6) {
            System.arraycopy(nArray2, (n6 + (n3 & 3)) * n11 + (n2 & 3), nArray3, n6 * n4, n4);
        }
        return nArray3;
    }
}

