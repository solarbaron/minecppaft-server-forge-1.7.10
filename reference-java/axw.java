/*
 * Decompiled with CFR 0.152.
 */
public class axw
extends axn {
    public axw(long l2, axn axn2) {
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
                ahu ahu2 = ahu.d(n10);
                if (n10 == ahu.C.ay) {
                    n9 = nArray[i3 + 1 + (i2 + 1 - 1) * (n4 + 2)];
                    n8 = nArray[i3 + 1 + 1 + (i2 + 1) * (n4 + 2)];
                    n7 = nArray[i3 + 1 - 1 + (i2 + 1) * (n4 + 2)];
                    n6 = nArray[i3 + 1 + (i2 + 1 + 1) * (n4 + 2)];
                    if (n9 == ahu.o.ay || n8 == ahu.o.ay || n7 == ahu.o.ay || n6 == ahu.o.ay) {
                        nArray2[i3 + i2 * n4] = ahu.D.ay;
                        continue;
                    }
                    nArray2[i3 + i2 * n4] = n10;
                    continue;
                }
                if (ahu2 != null && ahu2.l() == aik.class) {
                    n9 = nArray[i3 + 1 + (i2 + 1 - 1) * (n4 + 2)];
                    n8 = nArray[i3 + 1 + 1 + (i2 + 1) * (n4 + 2)];
                    n7 = nArray[i3 + 1 - 1 + (i2 + 1) * (n4 + 2)];
                    n6 = nArray[i3 + 1 + (i2 + 1 + 1) * (n4 + 2)];
                    if (!(this.c(n9) && this.c(n8) && this.c(n7) && this.c(n6))) {
                        nArray2[i3 + i2 * n4] = ahu.L.ay;
                        continue;
                    }
                    if (axw.b(n9) || axw.b(n8) || axw.b(n7) || axw.b(n6)) {
                        nArray2[i3 + i2 * n4] = ahu.E.ay;
                        continue;
                    }
                    nArray2[i3 + i2 * n4] = n10;
                    continue;
                }
                if (n10 == ahu.r.ay || n10 == ahu.W.ay || n10 == ahu.I.ay) {
                    this.a(nArray, nArray2, i3, i2, n4, n10, ahu.N.ay);
                    continue;
                }
                if (ahu2 != null && ahu2.j()) {
                    this.a(nArray, nArray2, i3, i2, n4, n10, ahu.O.ay);
                    continue;
                }
                if (n10 == ahu.Z.ay || n10 == ahu.aa.ay) {
                    n9 = nArray[i3 + 1 + (i2 + 1 - 1) * (n4 + 2)];
                    n8 = nArray[i3 + 1 + 1 + (i2 + 1) * (n4 + 2)];
                    n7 = nArray[i3 + 1 - 1 + (i2 + 1) * (n4 + 2)];
                    n6 = nArray[i3 + 1 + (i2 + 1 + 1) * (n4 + 2)];
                    if (axw.b(n9) || axw.b(n8) || axw.b(n7) || axw.b(n6)) {
                        nArray2[i3 + i2 * n4] = n10;
                        continue;
                    }
                    if (!(this.d(n9) && this.d(n8) && this.d(n7) && this.d(n6))) {
                        nArray2[i3 + i2 * n4] = ahu.q.ay;
                        continue;
                    }
                    nArray2[i3 + i2 * n4] = n10;
                    continue;
                }
                if (n10 != ahu.o.ay && n10 != ahu.M.ay && n10 != ahu.v.ay && n10 != ahu.u.ay) {
                    n9 = nArray[i3 + 1 + (i2 + 1 - 1) * (n4 + 2)];
                    n8 = nArray[i3 + 1 + 1 + (i2 + 1) * (n4 + 2)];
                    n7 = nArray[i3 + 1 - 1 + (i2 + 1) * (n4 + 2)];
                    n6 = nArray[i3 + 1 + (i2 + 1 + 1) * (n4 + 2)];
                    if (axw.b(n9) || axw.b(n8) || axw.b(n7) || axw.b(n6)) {
                        nArray2[i3 + i2 * n4] = ahu.E.ay;
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

    private void a(int[] nArray, int[] nArray2, int n2, int n3, int n4, int n5, int n6) {
        if (axw.b(n5)) {
            nArray2[n2 + n3 * n4] = n5;
            return;
        }
        int n7 = nArray[n2 + 1 + (n3 + 1 - 1) * (n4 + 2)];
        int n8 = nArray[n2 + 1 + 1 + (n3 + 1) * (n4 + 2)];
        int n9 = nArray[n2 + 1 - 1 + (n3 + 1) * (n4 + 2)];
        int n10 = nArray[n2 + 1 + (n3 + 1 + 1) * (n4 + 2)];
        nArray2[n2 + n3 * n4] = axw.b(n7) || axw.b(n8) || axw.b(n9) || axw.b(n10) ? n6 : n5;
    }

    private boolean c(int n2) {
        if (ahu.d(n2) != null && ahu.d(n2).l() == aik.class) {
            return true;
        }
        return n2 == ahu.L.ay || n2 == ahu.J.ay || n2 == ahu.K.ay || n2 == ahu.s.ay || n2 == ahu.t.ay || axw.b(n2);
    }

    private boolean d(int n2) {
        return ahu.d(n2) != null && ahu.d(n2) instanceof ail;
    }
}

