/*
 * Decompiled with CFR 0.152.
 */
public class yf
extends ym {
    public yf(ahb ahb2) {
        super(ahb2);
        this.ae = true;
    }

    @Override
    protected void aD() {
        super.aD();
        this.a(yj.d).a(0.2f);
    }

    @Override
    public boolean by() {
        return this.o.r != rd.a && this.o.b(this.C) && this.o.a((sa)this, this.C).isEmpty() && !this.o.d(this.C);
    }

    @Override
    public int aV() {
        return this.bX() * 3;
    }

    @Override
    public float d(float f2) {
        return 1.0f;
    }

    @Override
    protected String bP() {
        return "flame";
    }

    @Override
    protected ym bQ() {
        return new yf(this.o);
    }

    @Override
    protected adb u() {
        return ade.bs;
    }

    @Override
    protected void b(boolean bl2, int n2) {
        adb adb2 = this.u();
        if (adb2 != null && this.bX() > 1) {
            int n3 = this.Z.nextInt(4) - 2;
            if (n2 > 0) {
                n3 += this.Z.nextInt(n2 + 1);
            }
            for (int i2 = 0; i2 < n3; ++i2) {
                this.a(adb2, 1);
            }
        }
    }

    @Override
    public boolean al() {
        return false;
    }

    @Override
    protected int bR() {
        return super.bR() * 4;
    }

    @Override
    protected void bS() {
        this.h *= 0.9f;
    }

    @Override
    protected void bj() {
        this.w = 0.42f + (float)this.bX() * 0.1f;
        this.al = true;
    }

    @Override
    protected void b(float f2) {
    }

    @Override
    protected boolean bT() {
        return true;
    }

    @Override
    protected int bU() {
        return super.bU() + 2;
    }

    @Override
    protected String bV() {
        if (this.bX() > 1) {
            return "mob.magmacube.big";
        }
        return "mob.magmacube.small";
    }

    @Override
    public boolean P() {
        return false;
    }

    @Override
    protected boolean bW() {
        return true;
    }
}

