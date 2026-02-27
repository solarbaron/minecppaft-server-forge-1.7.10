/*
 * Decompiled with CFR 0.152.
 */
public class axf
extends axn {
    private ahu[] c = new ahu[]{ahu.q, ahu.q, ahu.q, ahu.X, ahu.X, ahu.p};
    private ahu[] d = new ahu[]{ahu.s, ahu.R, ahu.r, ahu.p, ahu.P, ahu.u};
    private ahu[] e = new ahu[]{ahu.s, ahu.r, ahu.t, ahu.p};
    private ahu[] f = new ahu[]{ahu.A, ahu.A, ahu.A, ahu.S};

    public axf(long l2, axn axn2, ahm ahm2) {
        super(l2);
        this.a = axn2;
        if (ahm2 == ahm.f) {
            this.c = new ahu[]{ahu.q, ahu.s, ahu.r, ahu.u, ahu.p, ahu.t};
        }
    }

    @Override
    public int[] a(int n2, int n3, int n4, int n5) {
        int[] nArray = this.a.a(n2, n3, n4, n5);
        int[] nArray2 = axl.a(n4 * n5);
        for (int i2 = 0; i2 < n5; ++i2) {
            for (int i3 = 0; i3 < n4; ++i3) {
                this.a((long)(i3 + n2), (long)(i2 + n3));
                int n6 = nArray[i3 + i2 * n4];
                int n7 = (n6 & 0xF00) >> 8;
                if (axf.b(n6 &= 0xFFFFF0FF)) {
                    nArray2[i3 + i2 * n4] = n6;
                    continue;
                }
                if (n6 == ahu.C.ay) {
                    nArray2[i3 + i2 * n4] = n6;
                    continue;
                }
                if (n6 == 1) {
                    if (n7 > 0) {
                        if (this.a(3) == 0) {
                            nArray2[i3 + i2 * n4] = ahu.ab.ay;
                            continue;
                        }
                        nArray2[i3 + i2 * n4] = ahu.aa.ay;
                        continue;
                    }
                    nArray2[i3 + i2 * n4] = this.c[this.a((int)this.c.length)].ay;
                    continue;
                }
                if (n6 == 2) {
                    if (n7 > 0) {
                        nArray2[i3 + i2 * n4] = ahu.J.ay;
                        continue;
                    }
                    nArray2[i3 + i2 * n4] = this.d[this.a((int)this.d.length)].ay;
                    continue;
                }
                if (n6 == 3) {
                    if (n7 > 0) {
                        nArray2[i3 + i2 * n4] = ahu.U.ay;
                        continue;
                    }
                    nArray2[i3 + i2 * n4] = this.e[this.a((int)this.e.length)].ay;
                    continue;
                }
                nArray2[i3 + i2 * n4] = n6 == 4 ? this.f[this.a((int)this.f.length)].ay : ahu.C.ay;
            }
        }
        return nArray2;
    }
}

