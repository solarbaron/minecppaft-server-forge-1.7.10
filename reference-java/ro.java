/*
 * Decompiled with CFR 0.152.
 */
public class ro {
    public static ro a = new ro("inFire").n();
    public static ro b = new ro("onFire").k().n();
    public static ro c = new ro("lava").n();
    public static ro d = new ro("inWall").k();
    public static ro e = new ro("drown").k();
    public static ro f = new ro("starve").k().m();
    public static ro g = new ro("cactus");
    public static ro h = new ro("fall").k();
    public static ro i = new ro("outOfWorld").k().l();
    public static ro j = new ro("generic").k();
    public static ro k = new ro("magic").k().t();
    public static ro l = new ro("wither").k();
    public static ro m = new ro("anvil");
    public static ro n = new ro("fallingBlock");
    private boolean p;
    private boolean q;
    private boolean r;
    private float s = 0.3f;
    private boolean t;
    private boolean u;
    private boolean v;
    private boolean w;
    private boolean x;
    public String o;

    public static ro a(sv sv2) {
        return new rp("mob", sv2);
    }

    public static ro a(yz yz2) {
        return new rp("player", yz2);
    }

    public static ro a(zc zc2, sa sa2) {
        return new rq("arrow", zc2, sa2).b();
    }

    public static ro a(ze ze2, sa sa2) {
        if (sa2 == null) {
            return new rq("onFire", ze2, ze2).n().b();
        }
        return new rq("fireball", ze2, sa2).n().b();
    }

    public static ro a(sa sa2, sa sa3) {
        return new rq("thrown", sa2, sa3).b();
    }

    public static ro b(sa sa2, sa sa3) {
        return new rq("indirectMagic", sa2, sa3).k().t();
    }

    public static ro a(sa sa2) {
        return new rp("thorns", sa2).t();
    }

    public static ro a(agw agw2) {
        if (agw2 != null && agw2.c() != null) {
            return new rp("explosion.player", agw2.c()).q().d();
        }
        return new ro("explosion").q().d();
    }

    public boolean a() {
        return this.u;
    }

    public ro b() {
        this.u = true;
        return this;
    }

    public boolean c() {
        return this.x;
    }

    public ro d() {
        this.x = true;
        return this;
    }

    public boolean e() {
        return this.p;
    }

    public float f() {
        return this.s;
    }

    public boolean g() {
        return this.q;
    }

    public boolean h() {
        return this.r;
    }

    protected ro(String string) {
        this.o = string;
    }

    public sa i() {
        return this.j();
    }

    public sa j() {
        return null;
    }

    protected ro k() {
        this.p = true;
        this.s = 0.0f;
        return this;
    }

    protected ro l() {
        this.q = true;
        return this;
    }

    protected ro m() {
        this.r = true;
        this.s = 0.0f;
        return this;
    }

    protected ro n() {
        this.t = true;
        return this;
    }

    public fj b(sv sv2) {
        sv sv3 = sv2.aX();
        String string = "death.attack." + this.o;
        String string2 = string + ".player";
        if (sv3 != null && dd.c(string2)) {
            return new fr(string2, sv2.c_(), sv3.c_());
        }
        return new fr(string, sv2.c_());
    }

    public boolean o() {
        return this.t;
    }

    public String p() {
        return this.o;
    }

    public ro q() {
        this.v = true;
        return this;
    }

    public boolean r() {
        return this.v;
    }

    public boolean s() {
        return this.w;
    }

    public ro t() {
        this.w = true;
        return this;
    }
}

