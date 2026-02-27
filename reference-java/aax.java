/*
 * Decompiled with CFR 0.152.
 */
public class aax
extends aay {
    private final rb a;
    private yz b;
    private int c;

    public aax(yz yz2, rb rb2, rb rb3, int n2, int n3, int n4) {
        super(rb3, n2, n3, n4);
        this.b = yz2;
        this.a = rb2;
    }

    @Override
    public boolean a(add add2) {
        return false;
    }

    @Override
    public add a(int n2) {
        if (this.e()) {
            this.c += Math.min(n2, this.d().b);
        }
        return super.a(n2);
    }

    @Override
    protected void a(add add2, int n2) {
        this.c += n2;
        this.b(add2);
    }

    @Override
    protected void b(add add2) {
        add2.a(this.b.o, this.b, this.c);
        this.c = 0;
        if (add2.b() == adb.a(ajn.ai)) {
            this.b.a(pc.h, 1);
        }
        if (add2.b() instanceof adn) {
            this.b.a(pc.i, 1);
        }
        if (add2.b() == adb.a(ajn.al)) {
            this.b.a(pc.j, 1);
        }
        if (add2.b() instanceof ada) {
            this.b.a(pc.l, 1);
        }
        if (add2.b() == ade.P) {
            this.b.a(pc.m, 1);
        }
        if (add2.b() == ade.aU) {
            this.b.a(pc.n, 1);
        }
        if (add2.b() instanceof adn && ((adn)add2.b()).i() != adc.a) {
            this.b.a(pc.o, 1);
        }
        if (add2.b() instanceof aeh) {
            this.b.a(pc.r, 1);
        }
        if (add2.b() == adb.a(ajn.bn)) {
            this.b.a(pc.E, 1);
        }
        if (add2.b() == adb.a(ajn.X)) {
            this.b.a(pc.G, 1);
        }
    }

    @Override
    public void a(yz yz2, add add2) {
        this.b(add2);
        for (int i2 = 0; i2 < this.a.a(); ++i2) {
            add add3 = this.a.a(i2);
            if (add3 == null) continue;
            this.a.a(i2, 1);
            if (!add3.b().u()) continue;
            add add4 = new add(add3.b().t());
            if (add3.b().l(add3) && this.b.bm.a(add4)) continue;
            if (this.a.a(i2) == null) {
                this.a.a(i2, add4);
                continue;
            }
            this.b.a(add4, false);
        }
    }
}

