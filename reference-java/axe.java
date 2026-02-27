/*
 * Decompiled with CFR 0.152.
 */
public class axe
extends axn {
    public axe(long l2, axn axn2) {
        super(l2);
        this.a = axn2;
    }

    @Override
    public int[] a(int n2, int n3, int n4, int n5) {
        int[] nArray = this.a.a(n2 - 1, n3 - 1, n4 + 2, n5 + 2);
        int[] nArray2 = axl.a(n4 * n5);
        for (int i2 = 0; i2 < n5; ++i2) {
            for (int i3 = 0; i3 < n4; ++i3) {
                int n6;
                int n7;
                int n8;
                int n9;
                this.a((long)(i3 + n2), (long)(i2 + n3));
                int n10 = nArray[i3 + 1 + (i2 + 1) * (n4 + 2)];
                if (this.a(nArray, nArray2, i3, i2, n4, n10, ahu.r.ay, ahu.I.ay) || this.b(nArray, nArray2, i3, i2, n4, n10, ahu.aa.ay, ahu.Z.ay) || this.b(nArray, nArray2, i3, i2, n4, n10, ahu.ab.ay, ahu.Z.ay) || this.b(nArray, nArray2, i3, i2, n4, n10, ahu.U.ay, ahu.t.ay)) continue;
                if (n10 == ahu.q.ay) {
                    n9 = nArray[i3 + 1 + (i2 + 1 - 1) * (n4 + 2)];
                    n8 = nArray[i3 + 1 + 1 + (i2 + 1) * (n4 + 2)];
                    n7 = nArray[i3 + 1 - 1 + (i2 + 1) * (n4 + 2)];
                    n6 = nArray[i3 + 1 + (i2 + 1 + 1) * (n4 + 2)];
                    if (n9 == ahu.A.ay || n8 == ahu.A.ay || n7 == ahu.A.ay || n6 == ahu.A.ay) {
                        nArray2[i3 + i2 * n4] = ahu.W.ay;
                        continue;
                    }
                    nArray2[i3 + i2 * n4] = n10;
                    continue;
                }
                if (n10 == ahu.u.ay) {
                    n9 = nArray[i3 + 1 + (i2 + 1 - 1) * (n4 + 2)];
                    n8 = nArray[i3 + 1 + 1 + (i2 + 1) * (n4 + 2)];
                    n7 = nArray[i3 + 1 - 1 + (i2 + 1) * (n4 + 2)];
                    n6 = nArray[i3 + 1 + (i2 + 1 + 1) * (n4 + 2)];
                    if (n9 == ahu.q.ay || n8 == ahu.q.ay || n7 == ahu.q.ay || n6 == ahu.q.ay || n9 == ahu.S.ay || n8 == ahu.S.ay || n7 == ahu.S.ay || n6 == ahu.S.ay || n9 == ahu.A.ay || n8 == ahu.A.ay || n7 == ahu.A.ay || n6 == ahu.A.ay) {
                        nArray2[i3 + i2 * n4] = ahu.p.ay;
                        continue;
                    }
                    if (n9 == ahu.J.ay || n6 == ahu.J.ay || n8 == ahu.J.ay || n7 == ahu.J.ay) {
                        nArray2[i3 + i2 * n4] = ahu.L.ay;
                        continue;
                    }
                    nArray2[i3 + i2 * n4] = n10;
                    continue;
                }
                nArray2[i3 + i2 * n4] = n10;
            }
        }
        return nArray2;
    }

    private boolean a(int[] nArray, int[] nArray2, int n2, int n3, int n4, int n5, int n6, int n7) {
        if (axe.a(n5, n6)) {
            int n8 = nArray[n2 + 1 + (n3 + 1 - 1) * (n4 + 2)];
            int n9 = nArray[n2 + 1 + 1 + (n3 + 1) * (n4 + 2)];
            int n10 = nArray[n2 + 1 - 1 + (n3 + 1) * (n4 + 2)];
            int n11 = nArray[n2 + 1 + (n3 + 1 + 1) * (n4 + 2)];
            nArray2[n2 + n3 * n4] = !this.b(n8, n6) || !this.b(n9, n6) || !this.b(n10, n6) || !this.b(n11, n6) ? n7 : n5;
            return true;
        }
        return false;
    }

    private boolean b(int[] nArray, int[] nArray2, int n2, int n3, int n4, int n5, int n6, int n7) {
        if (n5 == n6) {
            int n8 = nArray[n2 + 1 + (n3 + 1 - 1) * (n4 + 2)];
            int n9 = nArray[n2 + 1 + 1 + (n3 + 1) * (n4 + 2)];
            int n10 = nArray[n2 + 1 - 1 + (n3 + 1) * (n4 + 2)];
            int n11 = nArray[n2 + 1 + (n3 + 1 + 1) * (n4 + 2)];
            nArray2[n2 + n3 * n4] = !axe.a(n8, n6) || !axe.a(n9, n6) || !axe.a(n10, n6) || !axe.a(n11, n6) ? n7 : n5;
            return true;
        }
        return false;
    }

    private boolean b(int n2, int n3) {
        if (axe.a(n2, n3)) {
            return true;
        }
        if (ahu.d(n2) != null && ahu.d(n3) != null) {
            ahw ahw2;
            ahw ahw3 = ahu.d(n2).m();
            return ahw3 == (ahw2 = ahu.d(n3).m()) || ahw3 == ahw.c || ahw2 == ahw.c;
        }
        return false;
    }
}

