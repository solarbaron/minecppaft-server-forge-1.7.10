/*
 * Decompiled with CFR 0.152.
 */
public class adl
extends abh {
    protected final aji b;
    protected final String[] c;

    public adl(aji aji2, aji aji3, String[] stringArray) {
        super(aji2);
        this.b = aji3;
        this.c = stringArray;
        this.f(0);
        this.a(true);
    }

    @Override
    public int a(int n2) {
        return n2;
    }

    @Override
    public String a(add add2) {
        int n2 = add2.k();
        if (n2 < 0 || n2 >= this.c.length) {
            n2 = 0;
        }
        return super.a() + "." + this.c[n2];
    }
}

