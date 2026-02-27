/*
 * Decompiled with CFR 0.152.
 */
import org.apache.commons.lang3.tuple.ImmutablePair;

public class yk
extends yg {
    private int bp;

    public yk(ahb ahb2) {
        super(ahb2);
        this.a(0.3f, 0.7f);
    }

    @Override
    protected void aD() {
        super.aD();
        this.a(yj.a).a(8.0);
        this.a(yj.d).a(0.6f);
        this.a(yj.e).a(1.0);
    }

    @Override
    protected boolean g_() {
        return false;
    }

    @Override
    protected sa bR() {
        double d2 = 8.0;
        return this.o.b((sa)this, d2);
    }

    @Override
    protected String t() {
        return "mob.silverfish.say";
    }

    @Override
    protected String aT() {
        return "mob.silverfish.hit";
    }

    @Override
    protected String aU() {
        return "mob.silverfish.kill";
    }

    @Override
    public boolean a(ro ro2, float f2) {
        if (this.aw()) {
            return false;
        }
        if (this.bp <= 0 && (ro2 instanceof rp || ro2 == ro.k)) {
            this.bp = 20;
        }
        return super.a(ro2, f2);
    }

    @Override
    protected void a(sa sa2, float f2) {
        if (this.aB <= 0 && f2 < 1.2f && sa2.C.e > this.C.b && sa2.C.b < this.C.e) {
            this.aB = 20;
            this.n(sa2);
        }
    }

    @Override
    protected void a(int n2, int n3, int n4, aji aji2) {
        this.a("mob.silverfish.step", 0.15f, 1.0f);
    }

    @Override
    protected adb u() {
        return adb.d(0);
    }

    @Override
    public void h() {
        this.aM = this.y;
        super.h();
    }

    @Override
    protected void bq() {
        int n2;
        int n3;
        int n4;
        int n5;
        int n6;
        super.bq();
        if (this.o.E) {
            return;
        }
        if (this.bp > 0) {
            --this.bp;
            if (this.bp == 0) {
                n6 = qh.c(this.s);
                n5 = qh.c(this.t);
                n4 = qh.c(this.u);
                n3 = 0;
                int n7 = 0;
                while (n3 == 0 && n7 <= 5 && n7 >= -5) {
                    n2 = 0;
                    while (n3 == 0 && n2 <= 10 && n2 >= -10) {
                        int n8 = 0;
                        while (n3 == 0 && n8 <= 10 && n8 >= -10) {
                            if (this.o.a(n6 + n2, n5 + n7, n4 + n8) == ajn.aU) {
                                if (!this.o.O().b("mobGriefing")) {
                                    int n9 = this.o.e(n6 + n2, n5 + n7, n4 + n8);
                                    ImmutablePair immutablePair = amb.b(n9);
                                    this.o.d(n6 + n2, n5 + n7, n4 + n8, (aji)immutablePair.getLeft(), (Integer)immutablePair.getRight(), 3);
                                } else {
                                    this.o.a(n6 + n2, n5 + n7, n4 + n8, false);
                                }
                                ajn.aU.b(this.o, n6 + n2, n5 + n7, n4 + n8, 0);
                                if (this.Z.nextBoolean()) {
                                    n3 = 1;
                                    break;
                                }
                            }
                            n8 = n8 <= 0 ? 1 - n8 : 0 - n8;
                        }
                        n2 = n2 <= 0 ? 1 - n2 : 0 - n2;
                    }
                    n7 = n7 <= 0 ? 1 - n7 : 0 - n7;
                }
            }
        }
        if (this.bm == null && !this.bS()) {
            n6 = qh.c(this.s);
            n5 = qh.c(this.t + 0.5);
            n4 = qh.c(this.u);
            n3 = this.Z.nextInt(6);
            aji aji2 = this.o.a(n6 + q.b[n3], n5 + q.c[n3], n4 + q.d[n3]);
            n2 = this.o.e(n6 + q.b[n3], n5 + q.c[n3], n4 + q.d[n3]);
            if (amb.a(aji2)) {
                this.o.d(n6 + q.b[n3], n5 + q.c[n3], n4 + q.d[n3], ajn.aU, amb.a(aji2, n2), 3);
                this.s();
                this.B();
            } else {
                this.bQ();
            }
        } else if (this.bm != null && !this.bS()) {
            this.bm = null;
        }
    }

    @Override
    public float a(int n2, int n3, int n4) {
        if (this.o.a(n2, n3 - 1, n4) == ajn.b) {
            return 10.0f;
        }
        return super.a(n2, n3, n4);
    }

    @Override
    protected boolean j_() {
        return true;
    }

    @Override
    public boolean by() {
        if (super.by()) {
            yz yz2 = this.o.a((sa)this, 5.0);
            return yz2 == null;
        }
        return false;
    }

    @Override
    public sz bd() {
        return sz.c;
    }
}

