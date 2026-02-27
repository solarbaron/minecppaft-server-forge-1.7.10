/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

public class wc
extends ayl {
    private ahb a;
    private final List b = new ArrayList();
    private final List c = new ArrayList();
    private final List d = new ArrayList();
    private int e;

    public wc(String string) {
        super(string);
    }

    public wc(ahb ahb2) {
        super("villages");
        this.a = ahb2;
        this.c();
    }

    public void a(ahb ahb2) {
        this.a = ahb2;
        for (vz vz2 : this.d) {
            vz2.a(ahb2);
        }
    }

    public void a(int n2, int n3, int n4) {
        if (this.b.size() > 64) {
            return;
        }
        if (!this.d(n2, n3, n4)) {
            this.b.add(new r(n2, n3, n4));
        }
    }

    public void a() {
        ++this.e;
        for (vz vz2 : this.d) {
            vz2.a(this.e);
        }
        this.e();
        this.f();
        this.g();
        if (this.e % 400 == 0) {
            this.c();
        }
    }

    private void e() {
        Iterator iterator = this.d.iterator();
        while (iterator.hasNext()) {
            vz vz2 = (vz)iterator.next();
            if (!vz2.g()) continue;
            iterator.remove();
            this.c();
        }
    }

    public List b() {
        return this.d;
    }

    public vz a(int n2, int n3, int n4, int n5) {
        vz vz2 = null;
        float f2 = Float.MAX_VALUE;
        for (vz vz3 : this.d) {
            float f3;
            float f4 = vz3.a().e(n2, n3, n4);
            if (f4 >= f2 || f4 > (f3 = (float)(n5 + vz3.b())) * f3) continue;
            vz2 = vz3;
            f2 = f4;
        }
        return vz2;
    }

    private void f() {
        if (this.b.isEmpty()) {
            return;
        }
        this.a((r)this.b.remove(0));
    }

    private void g() {
        for (int i2 = 0; i2 < this.c.size(); ++i2) {
            vy vy2 = (vy)this.c.get(i2);
            boolean bl2 = false;
            for (vz vz2 : this.d) {
                int n2;
                int n3 = (int)vz2.a().e(vy2.a, vy2.b, vy2.c);
                if (n3 > (n2 = 32 + vz2.b()) * n2) continue;
                vz2.a(vy2);
                bl2 = true;
                break;
            }
            if (bl2) continue;
            vz vz3 = new vz(this.a);
            vz3.a(vy2);
            this.d.add(vz3);
            this.c();
        }
        this.c.clear();
    }

    private void a(r r2) {
        int n2 = 16;
        int n3 = 4;
        int n4 = 16;
        for (int i2 = r2.a - n2; i2 < r2.a + n2; ++i2) {
            for (int i3 = r2.b - n3; i3 < r2.b + n3; ++i3) {
                for (int i4 = r2.c - n4; i4 < r2.c + n4; ++i4) {
                    if (!this.e(i2, i3, i4)) continue;
                    vy vy2 = this.b(i2, i3, i4);
                    if (vy2 == null) {
                        this.c(i2, i3, i4);
                        continue;
                    }
                    vy2.f = this.e;
                }
            }
        }
    }

    private vy b(int n2, int n3, int n4) {
        for (Object object : this.c) {
            if (((vy)object).a != n2 || ((vy)object).c != n4 || Math.abs(((vy)object).b - n3) > 1) continue;
            return object;
        }
        for (Object object : this.d) {
            vy vy2 = ((vz)object).e(n2, n3, n4);
            if (vy2 == null) continue;
            return vy2;
        }
        return null;
    }

    private void c(int n2, int n3, int n4) {
        int n5 = ((akn)ajn.ao).e(this.a, n2, n3, n4);
        if (n5 == 0 || n5 == 2) {
            int n6;
            int n7 = 0;
            for (n6 = -5; n6 < 0; ++n6) {
                if (!this.a.i(n2 + n6, n3, n4)) continue;
                --n7;
            }
            for (n6 = 1; n6 <= 5; ++n6) {
                if (!this.a.i(n2 + n6, n3, n4)) continue;
                ++n7;
            }
            if (n7 != 0) {
                this.c.add(new vy(n2, n3, n4, n7 > 0 ? -2 : 2, 0, this.e));
            }
        } else {
            int n8;
            int n9 = 0;
            for (n8 = -5; n8 < 0; ++n8) {
                if (!this.a.i(n2, n3, n4 + n8)) continue;
                --n9;
            }
            for (n8 = 1; n8 <= 5; ++n8) {
                if (!this.a.i(n2, n3, n4 + n8)) continue;
                ++n9;
            }
            if (n9 != 0) {
                this.c.add(new vy(n2, n3, n4, 0, n9 > 0 ? -2 : 2, this.e));
            }
        }
    }

    private boolean d(int n2, int n3, int n4) {
        for (r r2 : this.b) {
            if (r2.a != n2 || r2.b != n3 || r2.c != n4) continue;
            return true;
        }
        return false;
    }

    private boolean e(int n2, int n3, int n4) {
        return this.a.a(n2, n3, n4) == ajn.ao;
    }

    @Override
    public void a(dh dh2) {
        this.e = dh2.f("Tick");
        dq dq2 = dh2.c("Villages", 10);
        for (int i2 = 0; i2 < dq2.c(); ++i2) {
            dh dh3 = dq2.b(i2);
            vz vz2 = new vz();
            vz2.a(dh3);
            this.d.add(vz2);
        }
    }

    @Override
    public void b(dh dh2) {
        dh2.a("Tick", this.e);
        dq dq2 = new dq();
        for (vz vz2 : this.d) {
            dh dh3 = new dh();
            vz2.b(dh3);
            dq2.a(dh3);
        }
        dh2.a("Villages", dq2);
    }
}

