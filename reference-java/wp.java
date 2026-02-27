/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class wp
extends wf {
    private final aae bq = new aae(new wq(this), 2, 1);
    public static final float[][] bp = new float[][]{{1.0f, 1.0f, 1.0f}, {0.85f, 0.5f, 0.2f}, {0.7f, 0.3f, 0.85f}, {0.4f, 0.6f, 0.85f}, {0.9f, 0.9f, 0.2f}, {0.5f, 0.8f, 0.1f}, {0.95f, 0.5f, 0.65f}, {0.3f, 0.3f, 0.3f}, {0.6f, 0.6f, 0.6f}, {0.3f, 0.5f, 0.6f}, {0.5f, 0.25f, 0.7f}, {0.2f, 0.3f, 0.7f}, {0.4f, 0.3f, 0.2f}, {0.4f, 0.5f, 0.2f}, {0.6f, 0.2f, 0.2f}, {0.1f, 0.1f, 0.1f}};
    private int br;
    private ud bs = new ud(this);

    public wp(ahb ahb2) {
        super(ahb2);
        this.a(0.9f, 1.3f);
        this.m().a(true);
        this.c.a(0, new uf(this));
        this.c.a(1, new uz(this, 1.25));
        this.c.a(2, new ua(this, 1.0));
        this.c.a(3, new vk(this, 1.1, ade.O, false));
        this.c.a(4, new uh(this, 1.1));
        this.c.a(5, this.bs);
        this.c.a(6, new vc(this, 1.0));
        this.c.a(7, new un(this, yz.class, 6.0f));
        this.c.a(8, new vb(this));
        this.bq.a(0, new add(ade.aR, 1, 0));
        this.bq.a(1, new add(ade.aR, 1, 0));
    }

    @Override
    protected boolean bk() {
        return true;
    }

    @Override
    protected void bn() {
        this.br = this.bs.f();
        super.bn();
    }

    @Override
    public void e() {
        if (this.o.E) {
            this.br = Math.max(0, this.br - 1);
        }
        super.e();
    }

    @Override
    protected void aD() {
        super.aD();
        this.a(yj.a).a(8.0);
        this.a(yj.d).a(0.23f);
    }

    @Override
    protected void c() {
        super.c();
        this.af.a(16, new Byte(0));
    }

    @Override
    protected void b(boolean bl2, int n2) {
        if (!this.ca()) {
            this.a(new add(adb.a(ajn.L), 1, this.bZ()), 0.0f);
        }
    }

    @Override
    protected adb u() {
        return adb.a(ajn.L);
    }

    @Override
    public boolean a(yz yz2) {
        add add2 = yz2.bm.h();
        if (add2 != null && add2.b() == ade.aZ && !this.ca() && !this.f()) {
            if (!this.o.E) {
                this.i(true);
                int n2 = 1 + this.Z.nextInt(3);
                for (int i2 = 0; i2 < n2; ++i2) {
                    xk xk2 = this.a(new add(adb.a(ajn.L), 1, this.bZ()), 1.0f);
                    xk2.w += (double)(this.Z.nextFloat() * 0.05f);
                    xk2.v += (double)((this.Z.nextFloat() - this.Z.nextFloat()) * 0.1f);
                    xk2.x += (double)((this.Z.nextFloat() - this.Z.nextFloat()) * 0.1f);
                }
            }
            add2.a(1, (sv)yz2);
            this.a("mob.sheep.shear", 1.0f, 1.0f);
        }
        return super.a(yz2);
    }

    @Override
    public void b(dh dh2) {
        super.b(dh2);
        dh2.a("Sheared", this.ca());
        dh2.a("Color", (byte)this.bZ());
    }

    @Override
    public void a(dh dh2) {
        super.a(dh2);
        this.i(dh2.n("Sheared"));
        this.s(dh2.d("Color"));
    }

    @Override
    protected String t() {
        return "mob.sheep.say";
    }

    @Override
    protected String aT() {
        return "mob.sheep.say";
    }

    @Override
    protected String aU() {
        return "mob.sheep.say";
    }

    @Override
    protected void a(int n2, int n3, int n4, aji aji2) {
        this.a("mob.sheep.step", 0.15f, 1.0f);
    }

    public int bZ() {
        return this.af.a(16) & 0xF;
    }

    public void s(int n2) {
        byte by2 = this.af.a(16);
        this.af.b(16, (byte)(by2 & 0xF0 | n2 & 0xF));
    }

    public boolean ca() {
        return (this.af.a(16) & 0x10) != 0;
    }

    public void i(boolean bl2) {
        byte by2 = this.af.a(16);
        if (bl2) {
            this.af.b(16, (byte)(by2 | 0x10));
        } else {
            this.af.b(16, (byte)(by2 & 0xFFFFFFEF));
        }
    }

    public static int a(Random random) {
        int n2 = random.nextInt(100);
        if (n2 < 5) {
            return 15;
        }
        if (n2 < 10) {
            return 7;
        }
        if (n2 < 15) {
            return 8;
        }
        if (n2 < 18) {
            return 12;
        }
        if (random.nextInt(500) == 0) {
            return 6;
        }
        return 0;
    }

    public wp b(rx rx2) {
        wp wp2 = (wp)rx2;
        wp wp3 = new wp(this.o);
        int n2 = this.a(this, wp2);
        wp3.s(15 - n2);
        return wp3;
    }

    @Override
    public void p() {
        this.i(false);
        if (this.f()) {
            this.a(60);
        }
    }

    @Override
    public sy a(sy sy2) {
        sy2 = super.a(sy2);
        this.s(wp.a(this.o.s));
        return sy2;
    }

    private int a(wf wf2, wf wf3) {
        int n2 = this.b(wf2);
        int n3 = this.b(wf3);
        this.bq.a(0).b(n2);
        this.bq.a(1).b(n3);
        add add2 = afe.a().a(this.bq, ((wp)wf2).o);
        int n4 = add2 != null && add2.b() == ade.aR ? add2.k() : (this.o.s.nextBoolean() ? n2 : n3);
        return n4;
    }

    private int b(wf wf2) {
        return 15 - ((wp)wf2).bZ();
    }

    @Override
    public /* synthetic */ rx a(rx rx2) {
        return this.b(rx2);
    }
}

