/*
 * Decompiled with CFR 0.152.
 */
public class adg
extends abh {
    private final alt b;

    public adg(alt alt2) {
        super(alt2);
        this.b = alt2;
        this.f(0);
        this.a(true);
    }

    @Override
    public int a(int n2) {
        return n2 | 4;
    }

    @Override
    public String a(add add2) {
        int n2 = add2.k();
        if (n2 < 0 || n2 >= this.b.e().length) {
            n2 = 0;
        }
        return super.a() + "." + this.b.e()[n2];
    }
}

