/*
 * Decompiled with CFR 0.152.
 */
public class akq
extends akm {
    private final cp P = new cm();

    @Override
    protected cp a(add add2) {
        return this.P;
    }

    @Override
    public aor a(ahb ahb2, int n2) {
        return new apc();
    }

    @Override
    protected void e(ahb ahb2, int n2, int n3, int n4) {
        cl cl2 = new cl(ahb2, n2, n3, n4);
        apb apb2 = (apb)cl2.j();
        if (apb2 == null) {
            return;
        }
        int n5 = apb2.i();
        if (n5 < 0) {
            ahb2.c(1001, n2, n3, n4, 0);
        } else {
            add add2;
            add add3 = apb2.a(n5);
            int n6 = ahb2.e(n2, n3, n4) & 7;
            rb rb2 = api.b(ahb2, n2 + q.b[n6], (double)(n3 + q.c[n6]), (double)(n4 + q.d[n6]));
            if (rb2 != null) {
                add2 = api.a(rb2, add3.m().a(1), q.a[n6]);
                if (add2 == null) {
                    add2 = add3.m();
                    if (--add2.b == 0) {
                        add2 = null;
                    }
                } else {
                    add2 = add3.m();
                }
            } else {
                add2 = this.P.a(cl2, add3);
                if (add2 != null && add2.b == 0) {
                    add2 = null;
                }
            }
            apb2.a(n5, add2);
        }
    }
}

