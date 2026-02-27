/*
 * Decompiled with CFR 0.152.
 */
public class uy
extends uc {
    boolean i;
    int j;

    public uy(sw sw2, boolean bl2) {
        super(sw2);
        this.a = sw2;
        this.i = bl2;
    }

    @Override
    public boolean b() {
        return this.i && this.j > 0 && super.b();
    }

    @Override
    public void c() {
        this.j = 20;
        this.e.a(this.a.o, this.b, this.c, this.d, true);
    }

    @Override
    public void d() {
        if (this.i) {
            this.e.a(this.a.o, this.b, this.c, this.d, false);
        }
    }

    @Override
    public void e() {
        --this.j;
        super.e();
    }
}

