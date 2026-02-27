/*
 * Decompiled with CFR 0.152.
 */
public class wh
extends wf {
    public wh(ahb ahb2) {
        super(ahb2);
        this.a(0.9f, 1.3f);
        this.m().a(true);
        this.c.a(0, new uf(this));
        this.c.a(1, new uz(this, 2.0));
        this.c.a(2, new ua(this, 1.0));
        this.c.a(3, new vk(this, 1.25, ade.O, false));
        this.c.a(4, new uh(this, 1.25));
        this.c.a(5, new vc(this, 1.0));
        this.c.a(6, new un(this, yz.class, 6.0f));
        this.c.a(7, new vb(this));
    }

    @Override
    public boolean bk() {
        return true;
    }

    @Override
    protected void aD() {
        super.aD();
        this.a(yj.a).a(10.0);
        this.a(yj.d).a(0.2f);
    }

    @Override
    protected String t() {
        return "mob.cow.say";
    }

    @Override
    protected String aT() {
        return "mob.cow.hurt";
    }

    @Override
    protected String aU() {
        return "mob.cow.hurt";
    }

    @Override
    protected void a(int n2, int n3, int n4, aji aji2) {
        this.a("mob.cow.step", 0.15f, 1.0f);
    }

    @Override
    protected float bf() {
        return 0.4f;
    }

    @Override
    protected adb u() {
        return ade.aA;
    }

    @Override
    protected void b(boolean bl2, int n2) {
        int n3;
        int n4 = this.Z.nextInt(3) + this.Z.nextInt(1 + n2);
        for (n3 = 0; n3 < n4; ++n3) {
            this.a(ade.aA, 1);
        }
        n4 = this.Z.nextInt(3) + 1 + this.Z.nextInt(1 + n2);
        for (n3 = 0; n3 < n4; ++n3) {
            if (this.al()) {
                this.a(ade.be, 1);
                continue;
            }
            this.a(ade.bd, 1);
        }
    }

    @Override
    public boolean a(yz yz2) {
        add add2 = yz2.bm.h();
        if (add2 != null && add2.b() == ade.ar && !yz2.bE.d) {
            if (add2.b-- == 1) {
                yz2.bm.a(yz2.bm.c, new add(ade.aB));
            } else if (!yz2.bm.a(new add(ade.aB))) {
                yz2.a(new add(ade.aB, 1, 0), false);
            }
            return true;
        }
        return super.a(yz2);
    }

    public wh b(rx rx2) {
        return new wh(this.o);
    }

    @Override
    public /* synthetic */ rx a(rx rx2) {
        return this.b(rx2);
    }
}

