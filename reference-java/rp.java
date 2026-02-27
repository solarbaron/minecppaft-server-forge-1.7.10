/*
 * Decompiled with CFR 0.152.
 */
public class rp
extends ro {
    protected sa p;

    public rp(String string, sa sa2) {
        super(string);
        this.p = sa2;
    }

    @Override
    public sa j() {
        return this.p;
    }

    @Override
    public fj b(sv sv2) {
        add add2 = this.p instanceof sv ? ((sv)this.p).be() : null;
        String string = "death.attack." + this.o;
        String string2 = string + ".item";
        if (add2 != null && add2.u() && dd.c(string2)) {
            return new fr(string2, sv2.c_(), this.p.c_(), add2.E());
        }
        return new fr(string, sv2.c_(), this.p.c_());
    }

    @Override
    public boolean r() {
        return this.p != null && this.p instanceof sv && !(this.p instanceof yz);
    }
}

