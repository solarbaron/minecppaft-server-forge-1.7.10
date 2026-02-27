/*
 * Decompiled with CFR 0.152.
 */
public class aau
extends aay {
    private final aas a;
    private yz b;
    private int c;
    private final agm d;

    public aau(yz yz2, agm agm2, aas aas2, int n2, int n3, int n4) {
        super(aas2, n2, n3, n4);
        this.b = yz2;
        this.d = agm2;
        this.a = aas2;
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
    }

    @Override
    public void a(yz yz2, add add2) {
        add add3;
        add add4;
        this.b(add2);
        agn agn2 = this.a.i();
        if (agn2 != null && (this.a(agn2, add4 = this.a.a(0), add3 = this.a.a(1)) || this.a(agn2, add3, add4))) {
            this.d.a(agn2);
            if (add4 != null && add4.b <= 0) {
                add4 = null;
            }
            if (add3 != null && add3.b <= 0) {
                add3 = null;
            }
            this.a.a(0, add4);
            this.a.a(1, add3);
        }
    }

    private boolean a(agn agn2, add add2, add add3) {
        add add4 = agn2.a();
        add add5 = agn2.b();
        if (add2 != null && add2.b() == add4.b()) {
            if (add5 != null && add3 != null && add5.b() == add3.b()) {
                add2.b -= add4.b;
                add3.b -= add5.b;
                return true;
            }
            if (add5 == null && add3 == null) {
                add2.b -= add4.b;
                return true;
            }
        }
        return false;
    }
}

