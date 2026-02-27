/*
 * Decompiled with CFR 0.152.
 */
public class aak
extends aay {
    private yz a;
    private int b;

    public aak(yz yz2, rb rb2, int n2, int n3, int n4) {
        super(rb2, n2, n3, n4);
        this.a = yz2;
    }

    @Override
    public boolean a(add add2) {
        return false;
    }

    @Override
    public add a(int n2) {
        if (this.e()) {
            this.b += Math.min(n2, this.d().b);
        }
        return super.a(n2);
    }

    @Override
    public void a(yz yz2, add add2) {
        this.b(add2);
        super.a(yz2, add2);
    }

    @Override
    protected void a(add add2, int n2) {
        this.b += n2;
        this.b(add2);
    }

    @Override
    protected void b(add add2) {
        add2.a(this.a.o, this.a, this.b);
        if (!this.a.o.E) {
            int n2;
            int n3 = this.b;
            float f2 = afa.a().b(add2);
            if (f2 == 0.0f) {
                n3 = 0;
            } else if (f2 < 1.0f) {
                n2 = qh.d((float)n3 * f2);
                if (n2 < qh.f((float)n3 * f2) && (float)Math.random() < (float)n3 * f2 - (float)n2) {
                    ++n2;
                }
                n3 = n2;
            }
            while (n3 > 0) {
                n2 = sq.a(n3);
                n3 -= n2;
                this.a.o.d(new sq(this.a.o, this.a.s, this.a.t + 0.5, this.a.u + 0.5, n2));
            }
        }
        this.b = 0;
        if (add2.b() == ade.j) {
            this.a.a(pc.k, 1);
        }
        if (add2.b() == ade.aQ) {
            this.a.a(pc.p, 1);
        }
    }
}

