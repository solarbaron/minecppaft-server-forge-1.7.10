/*
 * Decompiled with CFR 0.152.
 */
import com.google.common.collect.HashMultimap;
import com.google.common.collect.Multimap;
import java.text.DecimalFormat;
import java.util.Random;

public final class add {
    public static final DecimalFormat a = new DecimalFormat("#.###");
    public int b;
    public int c;
    private adb e;
    public dh d;
    private int f;
    private st g;

    public add(aji aji2) {
        this(aji2, 1);
    }

    public add(aji aji2, int n2) {
        this(aji2, n2, 0);
    }

    public add(aji aji2, int n2, int n3) {
        this(adb.a(aji2), n2, n3);
    }

    public add(adb adb2) {
        this(adb2, 1);
    }

    public add(adb adb2, int n2) {
        this(adb2, n2, 0);
    }

    public add(adb adb2, int n2, int n3) {
        this.e = adb2;
        this.b = n2;
        this.f = n3;
        if (this.f < 0) {
            this.f = 0;
        }
    }

    public static add a(dh dh2) {
        add add2 = new add();
        add2.c(dh2);
        return add2.b() != null ? add2 : null;
    }

    private add() {
    }

    public add a(int n2) {
        add add2 = new add(this.e, n2, this.f);
        if (this.d != null) {
            add2.d = (dh)this.d.b();
        }
        this.b -= n2;
        return add2;
    }

    public adb b() {
        return this.e;
    }

    public boolean a(yz yz2, ahb ahb2, int n2, int n3, int n4, int n5, float f2, float f3, float f4) {
        boolean bl2 = this.b().a(this, yz2, ahb2, n2, n3, n4, n5, f2, f3, f4);
        if (bl2) {
            yz2.a(pp.E[adb.b(this.e)], 1);
        }
        return bl2;
    }

    public float a(aji aji2) {
        return this.b().a(this, aji2);
    }

    public add a(ahb ahb2, yz yz2) {
        return this.b().a(this, ahb2, yz2);
    }

    public add b(ahb ahb2, yz yz2) {
        return this.b().b(this, ahb2, yz2);
    }

    public dh b(dh dh2) {
        dh2.a("id", (short)adb.b(this.e));
        dh2.a("Count", (byte)this.b);
        dh2.a("Damage", (short)this.f);
        if (this.d != null) {
            dh2.a("tag", this.d);
        }
        return dh2;
    }

    public void c(dh dh2) {
        this.e = adb.d(dh2.e("id"));
        this.b = dh2.d("Count");
        this.f = dh2.e("Damage");
        if (this.f < 0) {
            this.f = 0;
        }
        if (dh2.b("tag", 10)) {
            this.d = dh2.m("tag");
        }
    }

    public int e() {
        return this.b().m();
    }

    public boolean f() {
        return this.e() > 1 && (!this.g() || !this.i());
    }

    public boolean g() {
        if (this.e.o() <= 0) {
            return false;
        }
        return !this.p() || !this.q().n("Unbreakable");
    }

    public boolean h() {
        return this.e.n();
    }

    public boolean i() {
        return this.g() && this.f > 0;
    }

    public int j() {
        return this.f;
    }

    public int k() {
        return this.f;
    }

    public void b(int n2) {
        this.f = n2;
        if (this.f < 0) {
            this.f = 0;
        }
    }

    public int l() {
        return this.e.o();
    }

    public boolean a(int n2, Random random) {
        if (!this.g()) {
            return false;
        }
        if (n2 > 0) {
            int n3 = afv.a(aft.t.B, this);
            int n4 = 0;
            for (int i2 = 0; n3 > 0 && i2 < n2; ++i2) {
                if (!afr.a(this, n3, random)) continue;
                ++n4;
            }
            if ((n2 -= n4) <= 0) {
                return false;
            }
        }
        this.f += n2;
        return this.f > this.l();
    }

    public void a(int n2, sv sv2) {
        if (sv2 instanceof yz && ((yz)sv2).bE.d) {
            return;
        }
        if (!this.g()) {
            return;
        }
        if (this.a(n2, sv2.aI())) {
            sv2.a(this);
            --this.b;
            if (sv2 instanceof yz) {
                yz yz2 = (yz)sv2;
                yz2.a(pp.F[adb.b(this.e)], 1);
                if (this.b == 0 && this.b() instanceof abm) {
                    yz2.bG();
                }
            }
            if (this.b < 0) {
                this.b = 0;
            }
            this.f = 0;
        }
    }

    public void a(sv sv2, yz yz2) {
        boolean bl2 = this.e.a(this, sv2, (sv)yz2);
        if (bl2) {
            yz2.a(pp.E[adb.b(this.e)], 1);
        }
    }

    public void a(ahb ahb2, aji aji2, int n2, int n3, int n4, yz yz2) {
        boolean bl2 = this.e.a(this, ahb2, aji2, n2, n3, n4, yz2);
        if (bl2) {
            yz2.a(pp.E[adb.b(this.e)], 1);
        }
    }

    public boolean b(aji aji2) {
        return this.e.b(aji2);
    }

    public boolean a(yz yz2, sv sv2) {
        return this.e.a(this, yz2, sv2);
    }

    public add m() {
        add add2 = new add(this.e, this.b, this.f);
        if (this.d != null) {
            add2.d = (dh)this.d.b();
        }
        return add2;
    }

    public static boolean a(add add2, add add3) {
        if (add2 == null && add3 == null) {
            return true;
        }
        if (add2 == null || add3 == null) {
            return false;
        }
        if (add2.d == null && add3.d != null) {
            return false;
        }
        return add2.d == null || add2.d.equals(add3.d);
    }

    public static boolean b(add add2, add add3) {
        if (add2 == null && add3 == null) {
            return true;
        }
        if (add2 == null || add3 == null) {
            return false;
        }
        return add2.d(add3);
    }

    private boolean d(add add2) {
        if (this.b != add2.b) {
            return false;
        }
        if (this.e != add2.e) {
            return false;
        }
        if (this.f != add2.f) {
            return false;
        }
        if (this.d == null && add2.d != null) {
            return false;
        }
        return this.d == null || this.d.equals(add2.d);
    }

    public boolean a(add add2) {
        return this.e == add2.e && this.f == add2.f;
    }

    public String a() {
        return this.e.a(this);
    }

    public static add b(add add2) {
        return add2 == null ? null : add2.m();
    }

    public String toString() {
        return this.b + "x" + this.e.a() + "@" + this.f;
    }

    public void a(ahb ahb2, sa sa2, int n2, boolean bl2) {
        if (this.c > 0) {
            --this.c;
        }
        this.e.a(this, ahb2, sa2, n2, bl2);
    }

    public void a(ahb ahb2, yz yz2, int n2) {
        yz2.a(pp.D[adb.b(this.e)], n2);
        this.e.d(this, ahb2, yz2);
    }

    public int n() {
        return this.b().d_(this);
    }

    public aei o() {
        return this.b().d(this);
    }

    public void b(ahb ahb2, yz yz2, int n2) {
        this.b().a(this, ahb2, yz2, n2);
    }

    public boolean p() {
        return this.d != null;
    }

    public dh q() {
        return this.d;
    }

    public dq r() {
        if (this.d == null) {
            return null;
        }
        return this.d.c("ench", 10);
    }

    public void d(dh dh2) {
        this.d = dh2;
    }

    public String s() {
        dh dh2;
        String string = this.b().n(this);
        if (this.d != null && this.d.b("display", 10) && (dh2 = this.d.m("display")).b("Name", 8)) {
            string = dh2.j("Name");
        }
        return string;
    }

    public add c(String string) {
        if (this.d == null) {
            this.d = new dh();
        }
        if (!this.d.b("display", 10)) {
            this.d.a("display", new dh());
        }
        this.d.m("display").a("Name", string);
        return this;
    }

    public void t() {
        if (this.d == null) {
            return;
        }
        if (!this.d.b("display", 10)) {
            return;
        }
        dh dh2 = this.d.m("display");
        dh2.o("Name");
        if (dh2.d()) {
            this.d.o("display");
            if (this.d.d()) {
                this.d((dh)null);
            }
        }
    }

    public boolean u() {
        if (this.d == null) {
            return false;
        }
        if (!this.d.b("display", 10)) {
            return false;
        }
        return this.d.m("display").b("Name", 8);
    }

    public adq w() {
        return this.b().f(this);
    }

    public boolean x() {
        if (!this.b().e_(this)) {
            return false;
        }
        return !this.y();
    }

    public void a(aft aft2, int n2) {
        if (this.d == null) {
            this.d(new dh());
        }
        if (!this.d.b("ench", 9)) {
            this.d.a("ench", new dq());
        }
        dq dq2 = this.d.c("ench", 10);
        dh dh2 = new dh();
        dh2.a("id", (short)aft2.B);
        dh2.a("lvl", (short)((byte)n2));
        dq2.a(dh2);
    }

    public boolean y() {
        return this.d != null && this.d.b("ench", 9);
    }

    public void a(String string, dy dy2) {
        if (this.d == null) {
            this.d(new dh());
        }
        this.d.a(string, dy2);
    }

    public boolean z() {
        return this.b().v();
    }

    public boolean A() {
        return this.g != null;
    }

    public void a(st st2) {
        this.g = st2;
    }

    public st B() {
        return this.g;
    }

    public int C() {
        if (this.p() && this.d.b("RepairCost", 3)) {
            return this.d.f("RepairCost");
        }
        return 0;
    }

    public void c(int n2) {
        if (!this.p()) {
            this.d = new dh();
        }
        this.d.a("RepairCost", n2);
    }

    public Multimap D() {
        HashMultimap<String, tj> hashMultimap;
        if (this.p() && this.d.b("AttributeModifiers", 9)) {
            hashMultimap = HashMultimap.create();
            dq dq2 = this.d.c("AttributeModifiers", 10);
            for (int i2 = 0; i2 < dq2.c(); ++i2) {
                dh dh2 = dq2.b(i2);
                tj tj2 = yj.a(dh2);
                if (tj2.a().getLeastSignificantBits() == 0L || tj2.a().getMostSignificantBits() == 0L) continue;
                hashMultimap.put(dh2.j("AttributeName"), tj2);
            }
        } else {
            hashMultimap = this.b().k();
        }
        return hashMultimap;
    }

    public void a(adb adb2) {
        this.e = adb2;
    }

    public fj E() {
        fj fj2 = new fq("[").a(this.s()).a("]");
        if (this.e != null) {
            dh dh2 = new dh();
            this.b(dh2);
            fj2.b().a(new fl(fm.c, new fq(dh2.toString())));
            fj2.b().a(this.w().e);
        }
        return fj2;
    }
}

