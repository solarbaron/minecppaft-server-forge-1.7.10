/*
 * Decompiled with CFR 0.152.
 */
public class rq
extends rp {
    private sa q;

    public rq(String string, sa sa2, sa sa3) {
        super(string, sa2);
        this.q = sa3;
    }

    @Override
    public sa i() {
        return this.p;
    }

    @Override
    public sa j() {
        return this.q;
    }

    @Override
    public fj b(sv sv2) {
        fj fj2 = this.q == null ? this.p.c_() : this.q.c_();
        add add2 = this.q instanceof sv ? ((sv)this.q).be() : null;
        String string = "death.attack." + this.o;
        String string2 = string + ".item";
        if (add2 != null && add2.u() && dd.c(string2)) {
            return new fr(string2, sv2.c_(), fj2, add2.E());
        }
        return new fr(string, sv2.c_(), fj2);
    }
}

