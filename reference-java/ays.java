/*
 * Decompiled with CFR 0.152.
 */
import net.minecraft.server.MinecraftServer;

public class ays {
    private long a;
    private ahm b = ahm.b;
    private String c = "";
    private int d;
    private int e;
    private int f;
    private long g;
    private long h;
    private long i;
    private long j;
    private dh k;
    private int l;
    private String m;
    private int n;
    private boolean o;
    private int p;
    private boolean q;
    private int r;
    private ahk s;
    private boolean t;
    private boolean u;
    private boolean v;
    private boolean w;
    private agy x = new agy();

    protected ays() {
    }

    public ays(dh dh2) {
        this.a = dh2.g("RandomSeed");
        if (dh2.b("generatorName", 8)) {
            String string = dh2.j("generatorName");
            this.b = ahm.a(string);
            if (this.b == null) {
                this.b = ahm.b;
            } else if (this.b.f()) {
                int n2 = 0;
                if (dh2.b("generatorVersion", 99)) {
                    n2 = dh2.f("generatorVersion");
                }
                this.b = this.b.a(n2);
            }
            if (dh2.b("generatorOptions", 8)) {
                this.c = dh2.j("generatorOptions");
            }
        }
        this.s = ahk.a(dh2.f("GameType"));
        this.t = dh2.b("MapFeatures", 99) ? dh2.n("MapFeatures") : true;
        this.d = dh2.f("SpawnX");
        this.e = dh2.f("SpawnY");
        this.f = dh2.f("SpawnZ");
        this.g = dh2.g("Time");
        this.h = dh2.b("DayTime", 99) ? dh2.g("DayTime") : this.g;
        this.i = dh2.g("LastPlayed");
        this.j = dh2.g("SizeOnDisk");
        this.m = dh2.j("LevelName");
        this.n = dh2.f("version");
        this.p = dh2.f("rainTime");
        this.o = dh2.n("raining");
        this.r = dh2.f("thunderTime");
        this.q = dh2.n("thundering");
        this.u = dh2.n("hardcore");
        this.w = dh2.b("initialized", 99) ? dh2.n("initialized") : true;
        if (dh2.b("allowCommands", 99)) {
            this.v = dh2.n("allowCommands");
        } else {
            boolean bl2 = this.v = this.s == ahk.c;
        }
        if (dh2.b("Player", 10)) {
            this.k = dh2.m("Player");
            this.l = this.k.f("Dimension");
        }
        if (dh2.b("GameRules", 10)) {
            this.x.a(dh2.m("GameRules"));
        }
    }

    public ays(ahj ahj2, String string) {
        this.a = ahj2.d();
        this.s = ahj2.e();
        this.t = ahj2.g();
        this.m = string;
        this.u = ahj2.f();
        this.b = ahj2.h();
        this.c = ahj2.j();
        this.v = ahj2.i();
        this.w = false;
    }

    public ays(ays ays2) {
        this.a = ays2.a;
        this.b = ays2.b;
        this.c = ays2.c;
        this.s = ays2.s;
        this.t = ays2.t;
        this.d = ays2.d;
        this.e = ays2.e;
        this.f = ays2.f;
        this.g = ays2.g;
        this.h = ays2.h;
        this.i = ays2.i;
        this.j = ays2.j;
        this.k = ays2.k;
        this.l = ays2.l;
        this.m = ays2.m;
        this.n = ays2.n;
        this.p = ays2.p;
        this.o = ays2.o;
        this.r = ays2.r;
        this.q = ays2.q;
        this.u = ays2.u;
        this.v = ays2.v;
        this.w = ays2.w;
        this.x = ays2.x;
    }

    public dh a() {
        dh dh2 = new dh();
        this.a(dh2, this.k);
        return dh2;
    }

    public dh a(dh dh2) {
        dh dh3 = new dh();
        this.a(dh3, dh2);
        return dh3;
    }

    private void a(dh dh2, dh dh3) {
        dh2.a("RandomSeed", this.a);
        dh2.a("generatorName", this.b.a());
        dh2.a("generatorVersion", this.b.d());
        dh2.a("generatorOptions", this.c);
        dh2.a("GameType", this.s.a());
        dh2.a("MapFeatures", this.t);
        dh2.a("SpawnX", this.d);
        dh2.a("SpawnY", this.e);
        dh2.a("SpawnZ", this.f);
        dh2.a("Time", this.g);
        dh2.a("DayTime", this.h);
        dh2.a("SizeOnDisk", this.j);
        dh2.a("LastPlayed", MinecraftServer.ar());
        dh2.a("LevelName", this.m);
        dh2.a("version", this.n);
        dh2.a("rainTime", this.p);
        dh2.a("raining", this.o);
        dh2.a("thunderTime", this.r);
        dh2.a("thundering", this.q);
        dh2.a("hardcore", this.u);
        dh2.a("allowCommands", this.v);
        dh2.a("initialized", this.w);
        dh2.a("GameRules", this.x.a());
        if (dh3 != null) {
            dh2.a("Player", dh3);
        }
    }

    public long b() {
        return this.a;
    }

    public int c() {
        return this.d;
    }

    public int d() {
        return this.e;
    }

    public int e() {
        return this.f;
    }

    public long f() {
        return this.g;
    }

    public long g() {
        return this.h;
    }

    public dh i() {
        return this.k;
    }

    public int j() {
        return this.l;
    }

    public void b(long l2) {
        this.g = l2;
    }

    public void c(long l2) {
        this.h = l2;
    }

    public void a(int n2, int n3, int n4) {
        this.d = n2;
        this.e = n3;
        this.f = n4;
    }

    public String k() {
        return this.m;
    }

    public void a(String string) {
        this.m = string;
    }

    public int l() {
        return this.n;
    }

    public void e(int n2) {
        this.n = n2;
    }

    public boolean n() {
        return this.q;
    }

    public void a(boolean bl2) {
        this.q = bl2;
    }

    public int o() {
        return this.r;
    }

    public void f(int n2) {
        this.r = n2;
    }

    public boolean p() {
        return this.o;
    }

    public void b(boolean bl2) {
        this.o = bl2;
    }

    public int q() {
        return this.p;
    }

    public void g(int n2) {
        this.p = n2;
    }

    public ahk r() {
        return this.s;
    }

    public boolean s() {
        return this.t;
    }

    public void a(ahk ahk2) {
        this.s = ahk2;
    }

    public boolean t() {
        return this.u;
    }

    public ahm u() {
        return this.b;
    }

    public void a(ahm ahm2) {
        this.b = ahm2;
    }

    public String y() {
        return this.c;
    }

    public boolean v() {
        return this.v;
    }

    public boolean w() {
        return this.w;
    }

    public void d(boolean bl2) {
        this.w = bl2;
    }

    public agy x() {
        return this.x;
    }

    public void a(k k2) {
        k2.a("Level seed", new ayt(this));
        k2.a("Level generator", new ayu(this));
        k2.a("Level generator options", new ayv(this));
        k2.a("Level spawn location", new ayw(this));
        k2.a("Level time", new ayx(this));
        k2.a("Level dimension", new ayy(this));
        k2.a("Level storage version", new ayz(this));
        k2.a("Level weather", new aza(this));
        k2.a("Level game mode", new azb(this));
    }

    static /* synthetic */ ahm a(ays ays2) {
        return ays2.b;
    }

    static /* synthetic */ boolean b(ays ays2) {
        return ays2.t;
    }

    static /* synthetic */ String c(ays ays2) {
        return ays2.c;
    }

    static /* synthetic */ int d(ays ays2) {
        return ays2.d;
    }

    static /* synthetic */ int e(ays ays2) {
        return ays2.e;
    }

    static /* synthetic */ int f(ays ays2) {
        return ays2.f;
    }

    static /* synthetic */ long g(ays ays2) {
        return ays2.g;
    }

    static /* synthetic */ long h(ays ays2) {
        return ays2.h;
    }

    static /* synthetic */ int i(ays ays2) {
        return ays2.l;
    }

    static /* synthetic */ int j(ays ays2) {
        return ays2.n;
    }

    static /* synthetic */ int k(ays ays2) {
        return ays2.p;
    }

    static /* synthetic */ boolean l(ays ays2) {
        return ays2.o;
    }

    static /* synthetic */ int m(ays ays2) {
        return ays2.r;
    }

    static /* synthetic */ boolean n(ays ays2) {
        return ays2.q;
    }

    static /* synthetic */ ahk o(ays ays2) {
        return ays2.s;
    }

    static /* synthetic */ boolean p(ays ays2) {
        return ays2.u;
    }

    static /* synthetic */ boolean q(ays ays2) {
        return ays2.v;
    }
}

