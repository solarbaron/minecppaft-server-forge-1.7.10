/*
 * Decompiled with CFR 0.152.
 */
public class age
extends aft {
    protected age(int n2, int n3, afu afu2) {
        super(n2, n3, afu2);
        this.b("fishingSpeed");
    }

    @Override
    public int a(int n2) {
        return 15 + (n2 - 1) * 9;
    }

    @Override
    public int b(int n2) {
        return super.a(n2) + 50;
    }

    @Override
    public int b() {
        return 3;
    }
}

