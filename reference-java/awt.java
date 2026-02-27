/*
 * Decompiled with CFR 0.152.
 */
public class awt {
    public static final awt a = new awr(awv.b);
    public static final awt b = new awt(awv.c);
    public static final awt c = new awt(awv.l);
    public static final awt d = new awt(awv.o).g();
    public static final awt e = new awt(awv.m).f();
    public static final awt f = new awt(awv.h).f();
    public static final awt g = new awt(awv.h).f().o();
    public static final awt h = new aws(awv.n).n();
    public static final awt i = new aws(awv.f).n();
    public static final awt j = new awt(awv.i).g().s().n();
    public static final awt k = new awq(awv.i).n();
    public static final awt l = new awq(awv.i).g().n().i();
    public static final awt m = new awt(awv.e);
    public static final awt n = new awt(awv.e).g();
    public static final awt o = new awr(awv.b).n();
    public static final awt p = new awt(awv.d);
    public static final awt q = new awq(awv.b).n();
    public static final awt r = new awq(awv.e).g();
    public static final awt s = new awt(awv.b).s().p();
    public static final awt t = new awt(awv.b).p();
    public static final awt u = new awt(awv.f).g().s();
    public static final awt v = new awt(awv.i).n();
    public static final awt w = new awt(awv.g).s().p();
    public static final awt x = new awt(awv.g).p();
    public static final awt y = new awq(awv.j).i().s().f().n();
    public static final awt z = new awt(awv.j).f();
    public static final awt A = new awt(awv.i).s().n();
    public static final awt B = new awt(awv.k);
    public static final awt C = new awt(awv.i).n();
    public static final awt D = new awt(awv.i).n();
    public static final awt E = new aww(awv.b).o();
    public static final awt F = new awt(awv.b).n();
    public static final awt G = new awu(awv.e).f().n();
    public static final awt H = new awt(awv.m).o();
    private boolean I;
    private boolean J;
    private boolean K;
    private final awv L;
    private boolean M = true;
    private int N;
    private boolean O;

    public awt(awv awv2) {
        this.L = awv2;
    }

    public boolean d() {
        return false;
    }

    public boolean a() {
        return true;
    }

    public boolean b() {
        return true;
    }

    public boolean c() {
        return true;
    }

    private awt s() {
        this.K = true;
        return this;
    }

    protected awt f() {
        this.M = false;
        return this;
    }

    protected awt g() {
        this.I = true;
        return this;
    }

    public boolean h() {
        return this.I;
    }

    public awt i() {
        this.J = true;
        return this;
    }

    public boolean j() {
        return this.J;
    }

    public boolean k() {
        if (this.K) {
            return false;
        }
        return this.c();
    }

    public boolean l() {
        return this.M;
    }

    public int m() {
        return this.N;
    }

    protected awt n() {
        this.N = 1;
        return this;
    }

    protected awt o() {
        this.N = 2;
        return this;
    }

    protected awt p() {
        this.O = true;
        return this;
    }

    public boolean q() {
        return this.O;
    }

    public awv r() {
        return this.L;
    }
}

