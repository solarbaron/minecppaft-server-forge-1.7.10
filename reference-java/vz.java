/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.TreeMap;

public class vz {
    private ahb a;
    private final List b = new ArrayList();
    private final r c = new r(0, 0, 0);
    private final r d = new r(0, 0, 0);
    private int e;
    private int f;
    private int g;
    private int h;
    private int i;
    private TreeMap j = new TreeMap();
    private List k = new ArrayList();
    private int l;

    public vz() {
    }

    public vz(ahb ahb2) {
        this.a = ahb2;
    }

    public void a(ahb ahb2) {
        this.a = ahb2;
    }

    public void a(int n2) {
        azw azw2;
        int n3;
        this.g = n2;
        this.m();
        this.l();
        if (n2 % 20 == 0) {
            this.k();
        }
        if (n2 % 30 == 0) {
            this.j();
        }
        if (this.l < (n3 = this.h / 10) && this.b.size() > 20 && this.a.s.nextInt(7000) == 0 && (azw2 = this.a(qh.d(this.d.a), qh.d(this.d.b), qh.d(this.d.c), 2, 4, 2)) != null) {
            wt wt2 = new wt(this.a);
            wt2.b(azw2.a, azw2.b, azw2.c);
            this.a.d(wt2);
            ++this.l;
        }
    }

    private azw a(int n2, int n3, int n4, int n5, int n6, int n7) {
        for (int i2 = 0; i2 < 10; ++i2) {
            int n8;
            int n9;
            int n10 = n2 + this.a.s.nextInt(16) - 8;
            if (!this.a(n10, n9 = n3 + this.a.s.nextInt(6) - 3, n8 = n4 + this.a.s.nextInt(16) - 8) || !this.b(n10, n9, n8, n5, n6, n7)) continue;
            return azw.a(n10, n9, n8);
        }
        return null;
    }

    private boolean b(int n2, int n3, int n4, int n5, int n6, int n7) {
        if (!ahb.a(this.a, n2, n3 - 1, n4)) {
            return false;
        }
        int n8 = n2 - n5 / 2;
        int n9 = n4 - n7 / 2;
        for (int i2 = n8; i2 < n8 + n5; ++i2) {
            for (int i3 = n3; i3 < n3 + n6; ++i3) {
                for (int i4 = n9; i4 < n9 + n7; ++i4) {
                    if (!this.a.a(i2, i3, i4).r()) continue;
                    return false;
                }
            }
        }
        return true;
    }

    private void j() {
        List list = this.a.a(wt.class, azt.a(this.d.a - this.e, this.d.b - 4, this.d.c - this.e, this.d.a + this.e, this.d.b + 4, this.d.c + this.e));
        this.l = list.size();
    }

    private void k() {
        List list = this.a.a(yv.class, azt.a(this.d.a - this.e, this.d.b - 4, this.d.c - this.e, this.d.a + this.e, this.d.b + 4, this.d.c + this.e));
        this.h = list.size();
        if (this.h == 0) {
            this.j.clear();
        }
    }

    public r a() {
        return this.d;
    }

    public int b() {
        return this.e;
    }

    public int c() {
        return this.b.size();
    }

    public int d() {
        return this.g - this.f;
    }

    public int e() {
        return this.h;
    }

    public boolean a(int n2, int n3, int n4) {
        return this.d.e(n2, n3, n4) < (float)(this.e * this.e);
    }

    public List f() {
        return this.b;
    }

    public vy b(int n2, int n3, int n4) {
        vy vy2 = null;
        int n5 = Integer.MAX_VALUE;
        for (vy vy3 : this.b) {
            int n6 = vy3.b(n2, n3, n4);
            if (n6 >= n5) continue;
            vy2 = vy3;
            n5 = n6;
        }
        return vy2;
    }

    public vy c(int n2, int n3, int n4) {
        vy vy2 = null;
        int n5 = Integer.MAX_VALUE;
        for (vy vy3 : this.b) {
            int n6 = vy3.b(n2, n3, n4);
            n6 = n6 > 256 ? (n6 *= 1000) : vy3.f();
            if (n6 >= n5) continue;
            vy2 = vy3;
            n5 = n6;
        }
        return vy2;
    }

    public vy e(int n2, int n3, int n4) {
        if (this.d.e(n2, n3, n4) > (float)(this.e * this.e)) {
            return null;
        }
        for (vy vy2 : this.b) {
            if (vy2.a != n2 || vy2.c != n4 || Math.abs(vy2.b - n3) > 1) continue;
            return vy2;
        }
        return null;
    }

    public void a(vy vy2) {
        this.b.add(vy2);
        this.c.a += vy2.a;
        this.c.b += vy2.b;
        this.c.c += vy2.c;
        this.n();
        this.f = vy2.f;
    }

    public boolean g() {
        return this.b.isEmpty();
    }

    public void a(sv sv2) {
        for (wa wa2 : this.k) {
            if (wa2.a != sv2) continue;
            wa2.b = this.g;
            return;
        }
        this.k.add(new wa(this, sv2, this.g));
    }

    public sv b(sv sv2) {
        double d2 = Double.MAX_VALUE;
        wa wa2 = null;
        for (int i2 = 0; i2 < this.k.size(); ++i2) {
            wa wa3 = (wa)this.k.get(i2);
            double d3 = wa3.a.f(sv2);
            if (d3 > d2) continue;
            wa2 = wa3;
            d2 = d3;
        }
        return wa2 != null ? wa2.a : null;
    }

    public yz c(sv sv2) {
        double d2 = Double.MAX_VALUE;
        yz yz2 = null;
        for (String string : this.j.keySet()) {
            double d3;
            yz yz3;
            if (!this.d(string) || (yz3 = this.a.a(string)) == null || (d3 = yz3.f(sv2)) > d2) continue;
            yz2 = yz3;
            d2 = d3;
        }
        return yz2;
    }

    private void l() {
        Iterator iterator = this.k.iterator();
        while (iterator.hasNext()) {
            wa wa2 = (wa)iterator.next();
            if (wa2.a.Z() && Math.abs(this.g - wa2.b) <= 300) continue;
            iterator.remove();
        }
    }

    private void m() {
        boolean bl2 = false;
        boolean bl3 = this.a.s.nextInt(50) == 0;
        Iterator iterator = this.b.iterator();
        while (iterator.hasNext()) {
            vy vy2 = (vy)iterator.next();
            if (bl3) {
                vy2.d();
            }
            if (this.f(vy2.a, vy2.b, vy2.c) && Math.abs(this.g - vy2.f) <= 1200) continue;
            this.c.a -= vy2.a;
            this.c.b -= vy2.b;
            this.c.c -= vy2.c;
            bl2 = true;
            vy2.g = true;
            iterator.remove();
        }
        if (bl2) {
            this.n();
        }
    }

    private boolean f(int n2, int n3, int n4) {
        return this.a.a(n2, n3, n4) == ajn.ao;
    }

    private void n() {
        int n2 = this.b.size();
        if (n2 == 0) {
            this.d.b(0, 0, 0);
            this.e = 0;
            return;
        }
        this.d.b(this.c.a / n2, this.c.b / n2, this.c.c / n2);
        int n3 = 0;
        for (vy vy2 : this.b) {
            n3 = Math.max(vy2.b(this.d.a, this.d.b, this.d.c), n3);
        }
        this.e = Math.max(32, (int)Math.sqrt(n3) + 1);
    }

    public int a(String string) {
        Integer n2 = (Integer)this.j.get(string);
        if (n2 != null) {
            return n2;
        }
        return 0;
    }

    public int a(String string, int n2) {
        int n3 = this.a(string);
        int n4 = qh.a(n3 + n2, -30, 10);
        this.j.put(string, n4);
        return n4;
    }

    public boolean d(String string) {
        return this.a(string) <= -15;
    }

    public void a(dh dh2) {
        Object object;
        this.h = dh2.f("PopSize");
        this.e = dh2.f("Radius");
        this.l = dh2.f("Golems");
        this.f = dh2.f("Stable");
        this.g = dh2.f("Tick");
        this.i = dh2.f("MTick");
        this.d.a = dh2.f("CX");
        this.d.b = dh2.f("CY");
        this.d.c = dh2.f("CZ");
        this.c.a = dh2.f("ACX");
        this.c.b = dh2.f("ACY");
        this.c.c = dh2.f("ACZ");
        dq dq2 = dh2.c("Doors", 10);
        for (int i2 = 0; i2 < dq2.c(); ++i2) {
            dh dh3 = dq2.b(i2);
            object = new vy(dh3.f("X"), dh3.f("Y"), dh3.f("Z"), dh3.f("IDX"), dh3.f("IDZ"), dh3.f("TS"));
            this.b.add(object);
        }
        dq dq3 = dh2.c("Players", 10);
        for (int i3 = 0; i3 < dq3.c(); ++i3) {
            object = dq3.b(i3);
            this.j.put(((dh)object).j("Name"), ((dh)object).f("S"));
        }
    }

    public void b(dh dh2) {
        dh2.a("PopSize", this.h);
        dh2.a("Radius", this.e);
        dh2.a("Golems", this.l);
        dh2.a("Stable", this.f);
        dh2.a("Tick", this.g);
        dh2.a("MTick", this.i);
        dh2.a("CX", this.d.a);
        dh2.a("CY", this.d.b);
        dh2.a("CZ", this.d.c);
        dh2.a("ACX", this.c.a);
        dh2.a("ACY", this.c.b);
        dh2.a("ACZ", this.c.c);
        dq dq2 = new dq();
        for (Object object : this.b) {
            Object object2 = new dh();
            ((dh)object2).a("X", ((vy)object).a);
            ((dh)object2).a("Y", ((vy)object).b);
            ((dh)object2).a("Z", ((vy)object).c);
            ((dh)object2).a("IDX", ((vy)object).d);
            ((dh)object2).a("IDZ", ((vy)object).e);
            ((dh)object2).a("TS", ((vy)object).f);
            dq2.a((dy)object2);
        }
        dh2.a("Doors", dq2);
        dq dq3 = new dq();
        for (Object object2 : this.j.keySet()) {
            dh dh3 = new dh();
            dh3.a("Name", (String)object2);
            dh3.a("S", (Integer)this.j.get(object2));
            dq3.a(dh3);
        }
        dh2.a("Players", dq3);
    }

    public void h() {
        this.i = this.g;
    }

    public boolean i() {
        return this.i == 0 || this.g - this.i >= 3600;
    }

    public void b(int n2) {
        for (String string : this.j.keySet()) {
            this.a(string, n2);
        }
    }
}

