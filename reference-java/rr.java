/*
 * Decompiled with CFR 0.152.
 */
public class rr
extends rv {
    protected rr(int n2, boolean bl2, int n3) {
        super(n2, bl2, n3);
    }

    @Override
    public void a(sv sv2, tl tl2, int n2) {
        sv2.m(sv2.bs() - (float)(4 * (n2 + 1)));
        super.a(sv2, tl2, n2);
    }

    @Override
    public void b(sv sv2, tl tl2, int n2) {
        sv2.m(sv2.bs() + (float)(4 * (n2 + 1)));
        super.b(sv2, tl2, n2);
    }
}

