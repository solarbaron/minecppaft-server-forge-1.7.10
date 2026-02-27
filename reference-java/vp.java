/*
 * Decompiled with CFR 0.152.
 */
class vp
implements sj {
    final /* synthetic */ sj d;
    final /* synthetic */ vo e;

    vp(vo vo2, sj sj2) {
        this.e = vo2;
        this.d = sj2;
    }

    @Override
    public boolean a(sa sa2) {
        if (!(sa2 instanceof sv)) {
            return false;
        }
        if (this.d != null && !this.d.a(sa2)) {
            return false;
        }
        return this.e.a((sv)sa2, false);
    }
}

