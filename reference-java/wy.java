/*
 * Decompiled with CFR 0.152.
 */
public class wy
extends sa {
    public final wx a;
    public final String b;

    public wy(wx wx2, String string, float f2, float f3) {
        super(wx2.a());
        this.a(f2, f3);
        this.a = wx2;
        this.b = string;
    }

    @Override
    protected void c() {
    }

    @Override
    protected void a(dh dh2) {
    }

    @Override
    protected void b(dh dh2) {
    }

    @Override
    public boolean R() {
        return true;
    }

    @Override
    public boolean a(ro ro2, float f2) {
        if (this.aw()) {
            return false;
        }
        return this.a.a(this, ro2, f2);
    }

    @Override
    public boolean i(sa sa2) {
        return this == sa2 || this.a == sa2;
    }
}

