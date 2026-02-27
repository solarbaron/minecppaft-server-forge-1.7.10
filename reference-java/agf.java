/*
 * Decompiled with CFR 0.152.
 */
public class agf
extends aft {
    protected agf(int n2, int n3) {
        super(n2, n3, afu.g);
        this.b("knockback");
    }

    @Override
    public int a(int n2) {
        return 5 + 20 * (n2 - 1);
    }

    @Override
    public int b(int n2) {
        return super.a(n2) + 50;
    }

    @Override
    public int b() {
        return 2;
    }
}

