/*
 * Decompiled with CFR 0.152.
 */
public class rw {
    private int a;
    private int b;
    private int c;
    private boolean d;
    private boolean e;

    public rw(int n2, int n3) {
        this(n2, n3, 0);
    }

    public rw(int n2, int n3, int n4) {
        this(n2, n3, n4, false);
    }

    public rw(int n2, int n3, int n4, boolean bl2) {
        this.a = n2;
        this.b = n3;
        this.c = n4;
        this.e = bl2;
    }

    public rw(rw rw2) {
        this.a = rw2.a;
        this.b = rw2.b;
        this.c = rw2.c;
    }

    public void a(rw rw2) {
        if (this.a != rw2.a) {
            System.err.println("This method should only be called for matching effects!");
        }
        if (rw2.c > this.c) {
            this.c = rw2.c;
            this.b = rw2.b;
        } else if (rw2.c == this.c && this.b < rw2.b) {
            this.b = rw2.b;
        } else if (!rw2.e && this.e) {
            this.e = rw2.e;
        }
    }

    public int a() {
        return this.a;
    }

    public int b() {
        return this.b;
    }

    public int c() {
        return this.c;
    }

    public void a(boolean bl2) {
        this.d = bl2;
    }

    public boolean e() {
        return this.e;
    }

    public boolean a(sv sv2) {
        if (this.b > 0) {
            if (rv.a[this.a].a(this.b, this.c)) {
                this.b(sv2);
            }
            this.h();
        }
        return this.b > 0;
    }

    private int h() {
        return --this.b;
    }

    public void b(sv sv2) {
        if (this.b > 0) {
            rv.a[this.a].a(sv2, this.c);
        }
    }

    public String f() {
        return rv.a[this.a].a();
    }

    public int hashCode() {
        return this.a;
    }

    public String toString() {
        String string = "";
        string = this.c() > 0 ? this.f() + " x " + (this.c() + 1) + ", Duration: " + this.b() : this.f() + ", Duration: " + this.b();
        if (this.d) {
            string = string + ", Splash: true";
        }
        if (rv.a[this.a].i()) {
            return "(" + string + ")";
        }
        return string;
    }

    public boolean equals(Object object) {
        if (!(object instanceof rw)) {
            return false;
        }
        rw rw2 = (rw)object;
        return this.a == rw2.a && this.c == rw2.c && this.b == rw2.b && this.d == rw2.d && this.e == rw2.e;
    }

    public dh a(dh dh2) {
        dh2.a("Id", (byte)this.a());
        dh2.a("Amplifier", (byte)this.c());
        dh2.a("Duration", this.b());
        dh2.a("Ambient", this.e());
        return dh2;
    }

    public static rw b(dh dh2) {
        byte by2 = dh2.d("Id");
        if (by2 < 0 || by2 >= rv.a.length || rv.a[by2] == null) {
            return null;
        }
        byte by3 = dh2.d("Amplifier");
        int n2 = dh2.f("Duration");
        boolean bl2 = dh2.n("Ambient");
        return new rw(by2, n2, by3, bl2);
    }
}

