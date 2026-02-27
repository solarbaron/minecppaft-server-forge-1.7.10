/*
 * Decompiled with CFR 0.152.
 */
public class afs
extends aft {
    protected afs(int n2, int n3) {
        super(n2, n3, afu.h);
        this.b("digging");
    }

    @Override
    public int a(int n2) {
        return 1 + 10 * (n2 - 1);
    }

    @Override
    public int b(int n2) {
        return super.a(n2) + 50;
    }

    @Override
    public int b() {
        return 5;
    }

    @Override
    public boolean a(add add2) {
        if (add2.b() == ade.aZ) {
            return true;
        }
        return super.a(add2);
    }
}

