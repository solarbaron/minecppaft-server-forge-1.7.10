/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.Random;

public class ain
extends ahu {
    protected ahu aD;

    public ain(int n2, ahu ahu2) {
        super(n2);
        this.aD = ahu2;
        this.a(ahu2.ag, true);
        this.af = ahu2.af + " M";
        this.ai = ahu2.ai;
        this.ak = ahu2.ak;
        this.al = ahu2.al;
        this.am = ahu2.am;
        this.an = ahu2.an;
        this.ao = ahu2.ao;
        this.ap = ahu2.ap;
        this.aq = ahu2.aq;
        this.aw = ahu2.aw;
        this.ax = ahu2.ax;
        this.at = new ArrayList(ahu2.at);
        this.as = new ArrayList(ahu2.as);
        this.av = new ArrayList(ahu2.av);
        this.au = new ArrayList(ahu2.au);
        this.ao = ahu2.ao;
        this.ap = ahu2.ap;
        this.am = ahu2.am + 0.1f;
        this.an = ahu2.an + 0.2f;
    }

    @Override
    public void a(ahb ahb2, Random random, int n2, int n3) {
        this.aD.ar.a(ahb2, random, this, n2, n3);
    }

    @Override
    public void a(ahb ahb2, Random random, aji[] ajiArray, byte[] byArray, int n2, int n3, double d2) {
        this.aD.a(ahb2, random, ajiArray, byArray, n2, n3, d2);
    }

    @Override
    public float g() {
        return this.aD.g();
    }

    @Override
    public arc a(Random random) {
        return this.aD.a(random);
    }

    @Override
    public Class l() {
        return this.aD.l();
    }

    @Override
    public boolean a(ahu ahu2) {
        return this.aD.a(ahu2);
    }

    @Override
    public ahw m() {
        return this.aD.m();
    }
}

