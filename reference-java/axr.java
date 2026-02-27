/*
 * Decompiled with CFR 0.152.
 */
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class axr
extends axn {
    private static final Logger c = LogManager.getLogger();
    private axn d;

    public axr(long l2, axn axn2, axn axn3) {
        super(l2);
        this.a = axn2;
        this.d = axn3;
    }

    @Override
    public int[] a(int n2, int n3, int n4, int n5) {
        int[] nArray = this.a.a(n2 - 1, n3 - 1, n4 + 2, n5 + 2);
        int[] nArray2 = this.d.a(n2 - 1, n3 - 1, n4 + 2, n5 + 2);
        int[] nArray3 = axl.a(n4 * n5);
        for (int i2 = 0; i2 < n5; ++i2) {
            for (int i3 = 0; i3 < n4; ++i3) {
                boolean bl2;
                this.a((long)(i3 + n2), (long)(i2 + n3));
                int n6 = nArray[i3 + 1 + (i2 + 1) * (n4 + 2)];
                int n7 = nArray2[i3 + 1 + (i2 + 1) * (n4 + 2)];
                boolean bl3 = bl2 = (n7 - 2) % 29 == 0;
                if (n6 > 255) {
                    c.debug("old! " + n6);
                }
                if (n6 != 0 && n7 >= 2 && (n7 - 2) % 29 == 1 && n6 < 128) {
                    if (ahu.d(n6 + 128) != null) {
                        nArray3[i3 + i2 * n4] = n6 + 128;
                        continue;
                    }
                    nArray3[i3 + i2 * n4] = n6;
                    continue;
                }
                if (this.a(3) == 0 || bl2) {
                    int n8;
                    int n9 = n6;
                    if (n6 == ahu.q.ay) {
                        n9 = ahu.F.ay;
                    } else if (n6 == ahu.s.ay) {
                        n9 = ahu.G.ay;
                    } else if (n6 == ahu.P.ay) {
                        n9 = ahu.Q.ay;
                    } else if (n6 == ahu.R.ay) {
                        n9 = ahu.p.ay;
                    } else if (n6 == ahu.t.ay) {
                        n9 = ahu.H.ay;
                    } else if (n6 == ahu.U.ay) {
                        n9 = ahu.V.ay;
                    } else if (n6 == ahu.S.ay) {
                        n9 = ahu.T.ay;
                    } else if (n6 == ahu.p.ay) {
                        n9 = this.a(3) == 0 ? ahu.G.ay : ahu.s.ay;
                    } else if (n6 == ahu.A.ay) {
                        n9 = ahu.B.ay;
                    } else if (n6 == ahu.J.ay) {
                        n9 = ahu.K.ay;
                    } else if (n6 == ahu.o.ay) {
                        n9 = ahu.M.ay;
                    } else if (n6 == ahu.r.ay) {
                        n9 = ahu.W.ay;
                    } else if (n6 == ahu.X.ay) {
                        n9 = ahu.Y.ay;
                    } else if (axr.a(n6, ahu.aa.ay)) {
                        n9 = ahu.Z.ay;
                    } else if (n6 == ahu.M.ay && this.a(3) == 0) {
                        n8 = this.a(2);
                        n9 = n8 == 0 ? ahu.p.ay : ahu.s.ay;
                    }
                    if (bl2 && n9 != n6) {
                        n9 = ahu.d(n9 + 128) != null ? (n9 += 128) : n6;
                    }
                    if (n9 == n6) {
                        nArray3[i3 + i2 * n4] = n6;
                        continue;
                    }
                    n8 = nArray[i3 + 1 + (i2 + 1 - 1) * (n4 + 2)];
                    int n10 = nArray[i3 + 1 + 1 + (i2 + 1) * (n4 + 2)];
                    int n11 = nArray[i3 + 1 - 1 + (i2 + 1) * (n4 + 2)];
                    int n12 = nArray[i3 + 1 + (i2 + 1 + 1) * (n4 + 2)];
                    int n13 = 0;
                    if (axr.a(n8, n6)) {
                        ++n13;
                    }
                    if (axr.a(n10, n6)) {
                        ++n13;
                    }
                    if (axr.a(n11, n6)) {
                        ++n13;
                    }
                    if (axr.a(n12, n6)) {
                        ++n13;
                    }
                    if (n13 >= 3) {
                        nArray3[i3 + i2 * n4] = n9;
                        continue;
                    }
                    nArray3[i3 + i2 * n4] = n6;
                    continue;
                }
                nArray3[i3 + i2 * n4] = n6;
            }
        }
        return nArray3;
    }
}

