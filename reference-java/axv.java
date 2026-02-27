/*
 * Decompiled with CFR 0.152.
 */
public class axv
extends axn {
    private axn c;
    private axn d;

    public axv(long l2, axn axn2, axn axn3) {
        super(l2);
        this.c = axn2;
        this.d = axn3;
    }

    @Override
    public void a(long l2) {
        this.c.a(l2);
        this.d.a(l2);
        super.a(l2);
    }

    @Override
    public int[] a(int n2, int n3, int n4, int n5) {
        int[] nArray = this.c.a(n2, n3, n4, n5);
        int[] nArray2 = this.d.a(n2, n3, n4, n5);
        int[] nArray3 = axl.a(n4 * n5);
        for (int i2 = 0; i2 < n4 * n5; ++i2) {
            if (nArray[i2] == ahu.o.ay || nArray[i2] == ahu.M.ay) {
                nArray3[i2] = nArray[i2];
                continue;
            }
            if (nArray2[i2] == ahu.v.ay) {
                if (nArray[i2] == ahu.A.ay) {
                    nArray3[i2] = ahu.z.ay;
                    continue;
                }
                if (nArray[i2] == ahu.C.ay || nArray[i2] == ahu.D.ay) {
                    nArray3[i2] = ahu.D.ay;
                    continue;
                }
                nArray3[i2] = nArray2[i2] & 0xFF;
                continue;
            }
            nArray3[i2] = nArray[i2];
        }
        return nArray3;
    }
}

