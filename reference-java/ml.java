/*
 * Decompiled with CFR 0.152.
 */
public class ml
extends mx {
    private boolean c;
    private boolean d;
    private int e;
    private int f;

    public ml(ahb ahb2) {
        super(ahb2);
    }

    @Override
    public void a() {
        super.a();
        ++this.f;
        long l2 = this.a.I();
        long l3 = l2 / 24000L + 1L;
        if (!this.c && this.f > 20) {
            this.c = true;
            this.b.a.a(new gv(5, 0.0f));
        }
        boolean bl2 = this.d = l2 > 120500L;
        if (this.d) {
            ++this.e;
        }
        if (l2 % 24000L == 500L) {
            if (l3 <= 6L) {
                this.b.a(new fr("demo.day." + l3, new Object[0]));
            }
        } else if (l3 == 1L) {
            if (l2 == 100L) {
                this.b.a.a(new gv(5, 101.0f));
            } else if (l2 == 175L) {
                this.b.a.a(new gv(5, 102.0f));
            } else if (l2 == 250L) {
                this.b.a.a(new gv(5, 103.0f));
            }
        } else if (l3 == 5L && l2 % 24000L == 22000L) {
            this.b.a(new fr("demo.day.warning", new Object[0]));
        }
    }

    private void e() {
        if (this.e > 100) {
            this.b.a(new fr("demo.reminder", new Object[0]));
            this.e = 0;
        }
    }

    @Override
    public void a(int n2, int n3, int n4, int n5) {
        if (this.d) {
            this.e();
            return;
        }
        super.a(n2, n3, n4, n5);
    }

    @Override
    public void a(int n2, int n3, int n4) {
        if (this.d) {
            return;
        }
        super.a(n2, n3, n4);
    }

    @Override
    public boolean b(int n2, int n3, int n4) {
        if (this.d) {
            return false;
        }
        return super.b(n2, n3, n4);
    }

    @Override
    public boolean a(yz yz2, ahb ahb2, add add2) {
        if (this.d) {
            this.e();
            return false;
        }
        return super.a(yz2, ahb2, add2);
    }

    @Override
    public boolean a(yz yz2, ahb ahb2, add add2, int n2, int n3, int n4, int n5, float f2, float f3, float f4) {
        if (this.d) {
            this.e();
            return false;
        }
        return super.a(yz2, ahb2, add2, n2, n3, n4, n5, f2, f3, f4);
    }
}

