/*
 * Decompiled with CFR 0.152.
 */
public class agk
extends aft {
    protected agk(int n2, int n3) {
        super(n2, n3, afu.h);
        this.b("untouching");
    }

    @Override
    public int a(int n2) {
        return 15;
    }

    @Override
    public int b(int n2) {
        return super.a(n2) + 50;
    }

    @Override
    public int b() {
        return 1;
    }

    @Override
    public boolean a(aft aft2) {
        return super.a(aft2) && aft2.B != agk.u.B;
    }

    @Override
    public boolean a(add add2) {
        if (add2.b() == ade.aZ) {
            return true;
        }
        return super.a(add2);
    }
}

