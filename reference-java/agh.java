/*
 * Decompiled with CFR 0.152.
 */
public class agh
extends aft {
    public agh(int n2, int n3) {
        super(n2, n3, afu.f);
        this.b("oxygen");
    }

    @Override
    public int a(int n2) {
        return 10 * n2;
    }

    @Override
    public int b(int n2) {
        return this.a(n2) + 30;
    }

    @Override
    public int b() {
        return 3;
    }
}

