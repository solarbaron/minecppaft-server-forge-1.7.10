/*
 * Decompiled with CFR 0.152.
 */
public class gj
extends ft {
    private fj a;
    private boolean b = true;

    public gj() {
    }

    public gj(fj fj2) {
        this(fj2, true);
    }

    public gj(fj fj2, boolean bl2) {
        this.a = fj2;
        this.b = bl2;
    }

    @Override
    public void a(et et2) {
        this.a = fk.a(et2.c(Short.MAX_VALUE));
    }

    @Override
    public void b(et et2) {
        et2.a(fk.a(this.a));
    }

    public void a(fv fv2) {
        fv2.a(this);
    }

    @Override
    public String b() {
        return String.format("message='%s'", this.a);
    }

    public boolean d() {
        return this.b;
    }
}

