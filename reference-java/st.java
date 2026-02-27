/*
 * Decompiled with CFR 0.152.
 */
public class st
extends ss {
    private float e = 1.0f;

    public st(ahb ahb2) {
        super(ahb2);
    }

    public st(ahb ahb2, int n2, int n3, int n4, int n5) {
        super(ahb2, n2, n3, n4, n5);
        this.a(n5);
    }

    @Override
    protected void c() {
        this.z().a(2, 5);
        this.z().a(3, (Object)0);
    }

    @Override
    public boolean a(ro ro2, float f2) {
        if (this.aw()) {
            return false;
        }
        if (this.j() != null) {
            if (!this.o.E) {
                this.b(ro2.j(), false);
                this.a((add)null);
            }
            return true;
        }
        return super.a(ro2, f2);
    }

    @Override
    public int f() {
        return 9;
    }

    @Override
    public int i() {
        return 9;
    }

    @Override
    public void b(sa sa2) {
        this.b(sa2, true);
    }

    public void b(sa sa2, boolean bl2) {
        add add2 = this.j();
        if (sa2 instanceof yz) {
            yz yz2 = (yz)sa2;
            if (yz2.bE.d) {
                this.b(add2);
                return;
            }
        }
        if (bl2) {
            this.a(new add(ade.bD), 0.0f);
        }
        if (add2 != null && this.Z.nextFloat() < this.e) {
            add2 = add2.m();
            this.b(add2);
            this.a(add2, 0.0f);
        }
    }

    private void b(add add2) {
        if (add2 == null) {
            return;
        }
        if (add2.b() == ade.aY) {
            ayi ayi2 = ((adh)add2.b()).a(add2, this.o);
            ayi2.g.remove("frame-" + this.y());
        }
        add2.a((st)null);
    }

    public add j() {
        return this.z().f(2);
    }

    public void a(add add2) {
        if (add2 != null) {
            add2 = add2.m();
            add2.b = 1;
            add2.a(this);
        }
        this.z().b(2, add2);
        this.z().h(2);
    }

    public int k() {
        return this.z().a(3);
    }

    public void c(int n2) {
        this.z().b(3, (byte)(n2 % 4));
    }

    @Override
    public void b(dh dh2) {
        if (this.j() != null) {
            dh2.a("Item", this.j().b(new dh()));
            dh2.a("ItemRotation", (byte)this.k());
            dh2.a("ItemDropChance", this.e);
        }
        super.b(dh2);
    }

    @Override
    public void a(dh dh2) {
        dh dh3 = dh2.m("Item");
        if (dh3 != null && !dh3.d()) {
            this.a(add.a(dh3));
            this.c(dh2.d("ItemRotation"));
            if (dh2.b("ItemDropChance", 99)) {
                this.e = dh2.h("ItemDropChance");
            }
        }
        super.a(dh2);
    }

    @Override
    public boolean c(yz yz2) {
        if (this.j() == null) {
            add add2 = yz2.be();
            if (add2 != null && !this.o.E) {
                this.a(add2);
                if (!yz2.bE.d && --add2.b <= 0) {
                    yz2.bm.a(yz2.bm.c, null);
                }
            }
        } else if (!this.o.E) {
            this.c(this.k() + 1);
        }
        return true;
    }
}

