/*
 * Decompiled with CFR 0.152.
 */
public class fn {
    private fn a;
    private a b;
    private Boolean c;
    private Boolean d;
    private Boolean e;
    private Boolean f;
    private Boolean g;
    private fh h;
    private fl i;
    private static final fn j = new fo();

    public a a() {
        return this.b == null ? this.n().a() : this.b;
    }

    public boolean b() {
        return this.c == null ? this.n().b() : this.c.booleanValue();
    }

    public boolean c() {
        return this.d == null ? this.n().c() : this.d.booleanValue();
    }

    public boolean d() {
        return this.f == null ? this.n().d() : this.f.booleanValue();
    }

    public boolean e() {
        return this.e == null ? this.n().e() : this.e.booleanValue();
    }

    public boolean f() {
        return this.g == null ? this.n().f() : this.g.booleanValue();
    }

    public boolean g() {
        return this.c == null && this.d == null && this.f == null && this.e == null && this.g == null && this.b == null && this.h == null && this.i == null;
    }

    public fh h() {
        return this.h == null ? this.n().h() : this.h;
    }

    public fl i() {
        return this.i == null ? this.n().i() : this.i;
    }

    public fn a(a a2) {
        this.b = a2;
        return this;
    }

    public fn a(Boolean bl2) {
        this.c = bl2;
        return this;
    }

    public fn b(Boolean bl2) {
        this.d = bl2;
        return this;
    }

    public fn c(Boolean bl2) {
        this.f = bl2;
        return this;
    }

    public fn d(Boolean bl2) {
        this.e = bl2;
        return this;
    }

    public fn e(Boolean bl2) {
        this.g = bl2;
        return this;
    }

    public fn a(fh fh2) {
        this.h = fh2;
        return this;
    }

    public fn a(fl fl2) {
        this.i = fl2;
        return this;
    }

    public fn a(fn fn2) {
        this.a = fn2;
        return this;
    }

    private fn n() {
        return this.a == null ? j : this.a;
    }

    public String toString() {
        return "Style{hasParent=" + (this.a != null) + ", color=" + (Object)((Object)this.b) + ", bold=" + this.c + ", italic=" + this.d + ", underlined=" + this.e + ", obfuscated=" + this.g + ", clickEvent=" + this.h() + ", hoverEvent=" + this.i() + '}';
    }

    public boolean equals(Object object) {
        if (this == object) {
            return true;
        }
        if (object instanceof fn) {
            fn fn2 = (fn)object;
            return this.b() == fn2.b() && this.a() == fn2.a() && this.c() == fn2.c() && this.f() == fn2.f() && this.d() == fn2.d() && this.e() == fn2.e() && (this.h() != null ? this.h().equals(fn2.h()) : fn2.h() == null) && (this.i() != null ? this.i().equals(fn2.i()) : fn2.i() == null);
        }
        return false;
    }

    public int hashCode() {
        int n2 = this.b.hashCode();
        n2 = 31 * n2 + this.c.hashCode();
        n2 = 31 * n2 + this.d.hashCode();
        n2 = 31 * n2 + this.e.hashCode();
        n2 = 31 * n2 + this.f.hashCode();
        n2 = 31 * n2 + this.g.hashCode();
        n2 = 31 * n2 + this.h.hashCode();
        n2 = 31 * n2 + this.i.hashCode();
        return n2;
    }

    public fn l() {
        fn fn2 = new fn();
        fn2.c = this.c;
        fn2.d = this.d;
        fn2.f = this.f;
        fn2.e = this.e;
        fn2.g = this.g;
        fn2.b = this.b;
        fn2.h = this.h;
        fn2.i = this.i;
        fn2.a = this.a;
        return fn2;
    }

    public fn m() {
        fn fn2 = new fn();
        fn2.a(this.b());
        fn2.b(this.c());
        fn2.c(this.d());
        fn2.d(this.e());
        fn2.e(this.f());
        fn2.a(this.a());
        fn2.a(this.h());
        fn2.a(this.i());
        return fn2;
    }

    static /* synthetic */ Boolean a(fn fn2, Boolean bl2) {
        fn2.c = bl2;
        return fn2.c;
    }

    static /* synthetic */ Boolean b(fn fn2, Boolean bl2) {
        fn2.d = bl2;
        return fn2.d;
    }

    static /* synthetic */ Boolean c(fn fn2, Boolean bl2) {
        fn2.e = bl2;
        return fn2.e;
    }

    static /* synthetic */ Boolean d(fn fn2, Boolean bl2) {
        fn2.f = bl2;
        return fn2.f;
    }

    static /* synthetic */ Boolean e(fn fn2, Boolean bl2) {
        fn2.g = bl2;
        return fn2.g;
    }

    static /* synthetic */ a a(fn fn2, a a2) {
        fn2.b = a2;
        return fn2.b;
    }

    static /* synthetic */ fh a(fn fn2, fh fh2) {
        fn2.h = fh2;
        return fn2.h;
    }

    static /* synthetic */ fl a(fn fn2, fl fl2) {
        fn2.i = fl2;
        return fn2.i;
    }

    static /* synthetic */ Boolean b(fn fn2) {
        return fn2.c;
    }

    static /* synthetic */ Boolean c(fn fn2) {
        return fn2.d;
    }

    static /* synthetic */ Boolean d(fn fn2) {
        return fn2.e;
    }

    static /* synthetic */ Boolean e(fn fn2) {
        return fn2.f;
    }

    static /* synthetic */ Boolean f(fn fn2) {
        return fn2.g;
    }

    static /* synthetic */ a g(fn fn2) {
        return fn2.b;
    }

    static /* synthetic */ fh h(fn fn2) {
        return fn2.h;
    }

    static /* synthetic */ fl i(fn fn2) {
        return fn2.i;
    }
}

