/*
 * Decompiled with CFR 0.152.
 */
class zw
extends aay {
    final /* synthetic */ ahb a;
    final /* synthetic */ int b;
    final /* synthetic */ int c;
    final /* synthetic */ int d;
    final /* synthetic */ zu e;

    zw(zu zu2, rb rb2, int n2, int n3, int n4, ahb ahb2, int n5, int n6, int n7) {
        this.e = zu2;
        this.a = ahb2;
        this.b = n5;
        this.c = n6;
        this.d = n7;
        super(rb2, n2, n3, n4);
    }

    @Override
    public boolean a(add add2) {
        return false;
    }

    @Override
    public boolean a(yz yz2) {
        return (yz2.bE.d || yz2.bF >= this.e.a) && this.e.a > 0 && this.e();
    }

    @Override
    public void a(yz yz2, add add2) {
        if (!yz2.bE.d) {
            yz2.a(-this.e.a);
        }
        zu.a(this.e).a(0, null);
        if (zu.b(this.e) > 0) {
            add add3 = zu.a(this.e).a(1);
            if (add3 != null && add3.b > zu.b(this.e)) {
                add3.b -= zu.b(this.e);
                zu.a(this.e).a(1, add3);
            } else {
                zu.a(this.e).a(1, null);
            }
        } else {
            zu.a(this.e).a(1, null);
        }
        this.e.a = 0;
        if (!yz2.bE.d && !this.a.E && this.a.a(this.b, this.c, this.d) == ajn.bQ && yz2.aI().nextFloat() < 0.12f) {
            int n2 = this.a.e(this.b, this.c, this.d);
            int n3 = n2 & 3;
            int n4 = n2 >> 2;
            if (++n4 > 2) {
                this.a.f(this.b, this.c, this.d);
                this.a.c(1020, this.b, this.c, this.d, 0);
            } else {
                this.a.a(this.b, this.c, this.d, n3 | n4 << 2, 2);
                this.a.c(1021, this.b, this.c, this.d, 0);
            }
        } else if (!this.a.E) {
            this.a.c(1021, this.b, this.c, this.d, 0);
        }
    }
}

