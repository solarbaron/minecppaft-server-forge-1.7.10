/*
 * Decompiled with CFR 0.152.
 */
public class abr
extends abh {
    private final aji b;
    private String[] c;

    public abr(aji aji2, boolean bl2) {
        super(aji2);
        this.b = aji2;
        if (bl2) {
            this.f(0);
            this.a(true);
        }
    }

    @Override
    public int a(int n2) {
        return n2;
    }

    public abr a(String[] stringArray) {
        this.c = stringArray;
        return this;
    }

    @Override
    public String a(add add2) {
        if (this.c == null) {
            return super.a(add2);
        }
        int n2 = add2.k();
        if (n2 >= 0 && n2 < this.c.length) {
            return super.a(add2) + "." + this.c[n2];
        }
        return super.a(add2);
    }
}

