/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class aia {
    protected ahb a;
    protected Random b;
    protected int c;
    protected int d;
    protected arn e = new arj(4);
    protected arn f = new asj(ajn.m, 7);
    protected arn g = new asj(ajn.n, 6);
    protected arn h = new ase(ajn.d, 32);
    protected arn i = new ase(ajn.n, 32);
    protected arn j = new ase(ajn.q, 16);
    protected arn k = new ase(ajn.p, 8);
    protected arn l = new ase(ajn.o, 8);
    protected arn m = new ase(ajn.ax, 7);
    protected arn n = new ase(ajn.ag, 7);
    protected arn o = new ase(ajn.x, 6);
    protected aro p = new aro(ajn.N);
    protected arn q = new aro(ajn.P);
    protected arn r = new aro(ajn.Q);
    protected arn s = new aru();
    protected arn t = new ash();
    protected arn u = new arh();
    protected arn v = new aiz();
    protected int w;
    protected int x;
    protected int y = 2;
    protected int z = 1;
    protected int A;
    protected int B;
    protected int C;
    protected int D;
    protected int E = 1;
    protected int F = 3;
    protected int G = 1;
    protected int H;
    public boolean I = true;

    public void a(ahb ahb2, Random random, ahu ahu2, int n2, int n3) {
        if (this.a != null) {
            throw new RuntimeException("Already decorating!!");
        }
        this.a = ahb2;
        this.b = random;
        this.c = n2;
        this.d = n3;
        this.a(ahu2);
        this.a = null;
        this.b = null;
    }

    protected void a(ahu ahu2) {
        Object object;
        int n2;
        int n3;
        int n4;
        int n5;
        int n6;
        this.a();
        for (n6 = 0; n6 < this.F; ++n6) {
            n5 = this.c + this.b.nextInt(16) + 8;
            n4 = this.d + this.b.nextInt(16) + 8;
            this.f.a(this.a, this.b, n5, this.a.i(n5, n4), n4);
        }
        for (n6 = 0; n6 < this.G; ++n6) {
            n5 = this.c + this.b.nextInt(16) + 8;
            n4 = this.d + this.b.nextInt(16) + 8;
            this.e.a(this.a, this.b, n5, this.a.i(n5, n4), n4);
        }
        for (n6 = 0; n6 < this.E; ++n6) {
            n5 = this.c + this.b.nextInt(16) + 8;
            n4 = this.d + this.b.nextInt(16) + 8;
            this.g.a(this.a, this.b, n5, this.a.i(n5, n4), n4);
        }
        n6 = this.x;
        if (this.b.nextInt(10) == 0) {
            ++n6;
        }
        for (n5 = 0; n5 < n6; ++n5) {
            n4 = this.c + this.b.nextInt(16) + 8;
            n3 = this.d + this.b.nextInt(16) + 8;
            n2 = this.a.f(n4, n3);
            object = ahu2.a(this.b);
            ((arn)object).a(1.0, 1.0, 1.0);
            if (!((arn)object).a(this.a, this.b, n4, n2, n3)) continue;
            ((arc)object).b(this.a, this.b, n4, n2, n3);
        }
        for (n5 = 0; n5 < this.H; ++n5) {
            n4 = this.c + this.b.nextInt(16) + 8;
            n3 = this.d + this.b.nextInt(16) + 8;
            this.s.a(this.a, this.b, n4, this.a.f(n4, n3), n3);
        }
        for (n5 = 0; n5 < this.y; ++n5) {
            n4 = this.c + this.b.nextInt(16) + 8;
            object = ahu2.a(this.b, n4, n2 = this.b.nextInt(this.a.f(n4, n3 = this.d + this.b.nextInt(16) + 8) + 32), n3);
            alc alc2 = alc.e((String)object);
            if (alc2.o() == awt.a) continue;
            this.p.a(alc2, alc.f((String)object));
            this.p.a(this.a, this.b, n4, n2, n3);
        }
        for (n5 = 0; n5 < this.z; ++n5) {
            n4 = this.c + this.b.nextInt(16) + 8;
            n3 = this.d + this.b.nextInt(16) + 8;
            n2 = this.b.nextInt(this.a.f(n4, n3) * 2);
            object = ahu2.b(this.b);
            ((arn)object).a(this.a, this.b, n4, n2, n3);
        }
        for (n5 = 0; n5 < this.A; ++n5) {
            n4 = this.c + this.b.nextInt(16) + 8;
            n3 = this.d + this.b.nextInt(16) + 8;
            n2 = this.b.nextInt(this.a.f(n4, n3) * 2);
            new ark(ajn.I).a(this.a, this.b, n4, n2, n3);
        }
        for (n5 = 0; n5 < this.w; ++n5) {
            n4 = this.c + this.b.nextInt(16) + 8;
            n3 = this.d + this.b.nextInt(16) + 8;
            for (n2 = this.b.nextInt(this.a.f(n4, n3) * 2); n2 > 0 && this.a.c(n4, n2 - 1, n3); --n2) {
            }
            this.v.a(this.a, this.b, n4, n2, n3);
        }
        for (n5 = 0; n5 < this.B; ++n5) {
            if (this.b.nextInt(4) == 0) {
                n4 = this.c + this.b.nextInt(16) + 8;
                n3 = this.d + this.b.nextInt(16) + 8;
                n2 = this.a.f(n4, n3);
                this.q.a(this.a, this.b, n4, n2, n3);
            }
            if (this.b.nextInt(8) != 0) continue;
            n4 = this.c + this.b.nextInt(16) + 8;
            n3 = this.d + this.b.nextInt(16) + 8;
            n2 = this.b.nextInt(this.a.f(n4, n3) * 2);
            this.r.a(this.a, this.b, n4, n2, n3);
        }
        if (this.b.nextInt(4) == 0) {
            n5 = this.c + this.b.nextInt(16) + 8;
            n4 = this.d + this.b.nextInt(16) + 8;
            n3 = this.b.nextInt(this.a.f(n5, n4) * 2);
            this.q.a(this.a, this.b, n5, n3, n4);
        }
        if (this.b.nextInt(8) == 0) {
            n5 = this.c + this.b.nextInt(16) + 8;
            n4 = this.d + this.b.nextInt(16) + 8;
            n3 = this.b.nextInt(this.a.f(n5, n4) * 2);
            this.r.a(this.a, this.b, n5, n3, n4);
        }
        for (n5 = 0; n5 < this.C; ++n5) {
            n4 = this.c + this.b.nextInt(16) + 8;
            n3 = this.d + this.b.nextInt(16) + 8;
            n2 = this.b.nextInt(this.a.f(n4, n3) * 2);
            this.t.a(this.a, this.b, n4, n2, n3);
        }
        for (n5 = 0; n5 < 10; ++n5) {
            n4 = this.c + this.b.nextInt(16) + 8;
            n3 = this.d + this.b.nextInt(16) + 8;
            n2 = this.b.nextInt(this.a.f(n4, n3) * 2);
            this.t.a(this.a, this.b, n4, n2, n3);
        }
        if (this.b.nextInt(32) == 0) {
            n5 = this.c + this.b.nextInt(16) + 8;
            n4 = this.d + this.b.nextInt(16) + 8;
            n3 = this.b.nextInt(this.a.f(n5, n4) * 2);
            new asg().a(this.a, this.b, n5, n3, n4);
        }
        for (n5 = 0; n5 < this.D; ++n5) {
            n4 = this.c + this.b.nextInt(16) + 8;
            n3 = this.d + this.b.nextInt(16) + 8;
            n2 = this.b.nextInt(this.a.f(n4, n3) * 2);
            this.u.a(this.a, this.b, n4, n2, n3);
        }
        if (this.I) {
            for (n5 = 0; n5 < 50; ++n5) {
                n4 = this.c + this.b.nextInt(16) + 8;
                n3 = this.b.nextInt(this.b.nextInt(248) + 8);
                n2 = this.d + this.b.nextInt(16) + 8;
                new asm(ajn.i).a(this.a, this.b, n4, n3, n2);
            }
            for (n5 = 0; n5 < 20; ++n5) {
                n4 = this.c + this.b.nextInt(16) + 8;
                n3 = this.b.nextInt(this.b.nextInt(this.b.nextInt(240) + 8) + 8);
                n2 = this.d + this.b.nextInt(16) + 8;
                new asm(ajn.k).a(this.a, this.b, n4, n3, n2);
            }
        }
    }

    protected void a(int n2, arn arn2, int n3, int n4) {
        for (int i2 = 0; i2 < n2; ++i2) {
            int n5 = this.c + this.b.nextInt(16);
            int n6 = this.b.nextInt(n4 - n3) + n3;
            int n7 = this.d + this.b.nextInt(16);
            arn2.a(this.a, this.b, n5, n6, n7);
        }
    }

    protected void b(int n2, arn arn2, int n3, int n4) {
        for (int i2 = 0; i2 < n2; ++i2) {
            int n5 = this.c + this.b.nextInt(16);
            int n6 = this.b.nextInt(n4) + this.b.nextInt(n4) + (n3 - n4);
            int n7 = this.d + this.b.nextInt(16);
            arn2.a(this.a, this.b, n5, n6, n7);
        }
    }

    protected void a() {
        this.a(20, this.h, 0, 256);
        this.a(10, this.i, 0, 256);
        this.a(20, this.j, 0, 128);
        this.a(20, this.k, 0, 64);
        this.a(2, this.l, 0, 32);
        this.a(8, this.m, 0, 16);
        this.a(1, this.n, 0, 16);
        this.b(1, this.o, 16, 16);
    }
}

