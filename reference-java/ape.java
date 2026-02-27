/*
 * Decompiled with CFR 0.152.
 */
public class ape
extends aor {
    public float a;
    public float i;
    public int j;
    private int k;

    @Override
    public void h() {
        double d2;
        super.h();
        if (++this.k % 20 * 4 == 0) {
            this.b.c(this.c, this.d, this.e, ajn.bB, 1, this.j);
        }
        this.i = this.a;
        float f2 = 0.1f;
        if (this.j > 0 && this.a == 0.0f) {
            double d3 = (double)this.c + 0.5;
            d2 = (double)this.e + 0.5;
            this.b.a(d3, (double)this.d + 0.5, d2, "random.chestopen", 0.5f, this.b.s.nextFloat() * 0.1f + 0.9f);
        }
        if (this.j == 0 && this.a > 0.0f || this.j > 0 && this.a < 1.0f) {
            float f3;
            float f4 = this.a;
            this.a = this.j > 0 ? (this.a += f2) : (this.a -= f2);
            if (this.a > 1.0f) {
                this.a = 1.0f;
            }
            if (this.a < (f3 = 0.5f) && f4 >= f3) {
                d2 = (double)this.c + 0.5;
                double d4 = (double)this.e + 0.5;
                this.b.a(d2, (double)this.d + 0.5, d4, "random.chestclosed", 0.5f, this.b.s.nextFloat() * 0.1f + 0.9f);
            }
            if (this.a < 0.0f) {
                this.a = 0.0f;
            }
        }
    }

    @Override
    public boolean c(int n2, int n3) {
        if (n2 == 1) {
            this.j = n3;
            return true;
        }
        return super.c(n2, n3);
    }

    @Override
    public void s() {
        this.u();
        super.s();
    }

    public void a() {
        ++this.j;
        this.b.c(this.c, this.d, this.e, ajn.bB, 1, this.j);
    }

    public void b() {
        --this.j;
        this.b.c(this.c, this.d, this.e, ajn.bB, 1, this.j);
    }

    public boolean a(yz yz2) {
        if (this.b.o(this.c, this.d, this.e) != this) {
            return false;
        }
        return !(yz2.e((double)this.c + 0.5, (double)this.d + 0.5, (double)this.e + 0.5) > 64.0);
    }
}

