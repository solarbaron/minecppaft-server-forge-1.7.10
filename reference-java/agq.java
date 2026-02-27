/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.List;

public abstract class agq {
    public int b = 20;
    private String a = "Pig";
    private List e;
    private agr f;
    public double c;
    public double d;
    private int g = 200;
    private int h = 800;
    private int i = 4;
    private sa j;
    private int k = 6;
    private int l = 16;
    private int m = 4;

    public String e() {
        if (this.i() == null) {
            if (this.a.equals("Minecart")) {
                this.a = "MinecartRideable";
            }
            return this.a;
        }
        return this.i().c;
    }

    public void a(String string) {
        this.a = string;
    }

    public boolean f() {
        return this.a().a((double)this.b() + 0.5, (double)this.c() + 0.5, (double)this.d() + 0.5, (double)this.l) != null;
    }

    public void g() {
        if (!this.f()) {
            return;
        }
        if (this.a().E) {
            double d2 = (float)this.b() + this.a().s.nextFloat();
            double d3 = (float)this.c() + this.a().s.nextFloat();
            double d4 = (float)this.d() + this.a().s.nextFloat();
            this.a().a("smoke", d2, d3, d4, 0.0, 0.0, 0.0);
            this.a().a("flame", d2, d3, d4, 0.0, 0.0, 0.0);
            if (this.b > 0) {
                --this.b;
            }
            this.d = this.c;
            this.c = (this.c + (double)(1000.0f / ((float)this.b + 200.0f))) % 360.0;
        } else {
            if (this.b == -1) {
                this.j();
            }
            if (this.b > 0) {
                --this.b;
                return;
            }
            boolean bl2 = false;
            for (int i2 = 0; i2 < this.i; ++i2) {
                sa sa2 = sg.a(this.e(), this.a());
                if (sa2 == null) {
                    return;
                }
                int n2 = this.a().a(sa2.getClass(), azt.a(this.b(), this.c(), this.d(), this.b() + 1, this.c() + 1, this.d() + 1).b(this.m * 2, 4.0, this.m * 2)).size();
                if (n2 >= this.k) {
                    this.j();
                    return;
                }
                double d5 = (double)this.b() + (this.a().s.nextDouble() - this.a().s.nextDouble()) * (double)this.m;
                double d6 = this.c() + this.a().s.nextInt(3) - 1;
                double d7 = (double)this.d() + (this.a().s.nextDouble() - this.a().s.nextDouble()) * (double)this.m;
                sw sw2 = sa2 instanceof sw ? (sw)sa2 : null;
                sa2.b(d5, d6, d7, this.a().s.nextFloat() * 360.0f, 0.0f);
                if (sw2 != null && !sw2.by()) continue;
                this.a(sa2);
                this.a().c(2004, this.b(), this.c(), this.d(), 0);
                if (sw2 != null) {
                    sw2.s();
                }
                bl2 = true;
            }
            if (bl2) {
                this.j();
            }
        }
    }

    public sa a(sa sa2) {
        if (this.i() != null) {
            Object object;
            Object object2 = new dh();
            sa2.d((dh)object2);
            for (Object object3 : this.i().b.c()) {
                object = this.i().b.a((String)object3);
                ((dh)object2).a((String)object3, ((dy)object).b());
            }
            sa2.f((dh)object2);
            if (sa2.o != null) {
                sa2.o.d(sa2);
            }
            Object object4 = sa2;
            while (((dh)object2).b("Riding", 10)) {
                Object object3;
                object3 = ((dh)object2).m("Riding");
                object = sg.a(((dh)object3).j("id"), sa2.o);
                if (object != null) {
                    dh dh2 = new dh();
                    ((sa)object).d(dh2);
                    for (String string : ((dh)object3).c()) {
                        dy dy2 = ((dh)object3).a(string);
                        dh2.a(string, dy2.b());
                    }
                    ((sa)object).f(dh2);
                    ((sa)object).b(((sa)object4).s, ((sa)object4).t, ((sa)object4).u, ((sa)object4).y, ((sa)object4).z);
                    if (sa2.o != null) {
                        sa2.o.d((sa)object);
                    }
                    ((sa)object4).a((sa)object);
                }
                object4 = object;
                object2 = object3;
            }
        } else if (sa2 instanceof sv && sa2.o != null) {
            ((sw)sa2).a((sy)null);
            this.a().d(sa2);
        }
        return sa2;
    }

    private void j() {
        this.b = this.h <= this.g ? this.g : this.g + this.a().s.nextInt(this.h - this.g);
        if (this.e != null && this.e.size() > 0) {
            this.a((agr)qv.a(this.a().s, this.e));
        }
        this.a(1);
    }

    public void a(dh dh2) {
        this.a = dh2.j("EntityId");
        this.b = dh2.e("Delay");
        if (dh2.b("SpawnPotentials", 9)) {
            this.e = new ArrayList();
            dq dq2 = dh2.c("SpawnPotentials", 10);
            for (int i2 = 0; i2 < dq2.c(); ++i2) {
                this.e.add(new agr(this, dq2.b(i2)));
            }
        } else {
            this.e = null;
        }
        if (dh2.b("SpawnData", 10)) {
            this.a(new agr(this, dh2.m("SpawnData"), this.a));
        } else {
            this.a((agr)null);
        }
        if (dh2.b("MinSpawnDelay", 99)) {
            this.g = dh2.e("MinSpawnDelay");
            this.h = dh2.e("MaxSpawnDelay");
            this.i = dh2.e("SpawnCount");
        }
        if (dh2.b("MaxNearbyEntities", 99)) {
            this.k = dh2.e("MaxNearbyEntities");
            this.l = dh2.e("RequiredPlayerRange");
        }
        if (dh2.b("SpawnRange", 99)) {
            this.m = dh2.e("SpawnRange");
        }
        if (this.a() != null && this.a().E) {
            this.j = null;
        }
    }

    public void b(dh dh2) {
        dh2.a("EntityId", this.e());
        dh2.a("Delay", (short)this.b);
        dh2.a("MinSpawnDelay", (short)this.g);
        dh2.a("MaxSpawnDelay", (short)this.h);
        dh2.a("SpawnCount", (short)this.i);
        dh2.a("MaxNearbyEntities", (short)this.k);
        dh2.a("RequiredPlayerRange", (short)this.l);
        dh2.a("SpawnRange", (short)this.m);
        if (this.i() != null) {
            dh2.a("SpawnData", this.i().b.b());
        }
        if (this.i() != null || this.e != null && this.e.size() > 0) {
            dq dq2 = new dq();
            if (this.e != null && this.e.size() > 0) {
                for (agr agr2 : this.e) {
                    dq2.a(agr2.a());
                }
            } else {
                dq2.a(this.i().a());
            }
            dh2.a("SpawnPotentials", dq2);
        }
    }

    public boolean b(int n2) {
        if (n2 == 1 && this.a().E) {
            this.b = this.g;
            return true;
        }
        return false;
    }

    public agr i() {
        return this.f;
    }

    public void a(agr agr2) {
        this.f = agr2;
    }

    public abstract void a(int var1);

    public abstract ahb a();

    public abstract int b();

    public abstract int c();

    public abstract int d();
}

