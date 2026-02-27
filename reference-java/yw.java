/*
 * Decompiled with CFR 0.152.
 */
public class yw {
    public boolean a;
    public boolean b;
    public boolean c;
    public boolean d;
    public boolean e = true;
    private float f = 0.05f;
    private float g = 0.1f;

    public void a(dh dh2) {
        dh dh3 = new dh();
        dh3.a("invulnerable", this.a);
        dh3.a("flying", this.b);
        dh3.a("mayfly", this.c);
        dh3.a("instabuild", this.d);
        dh3.a("mayBuild", this.e);
        dh3.a("flySpeed", this.f);
        dh3.a("walkSpeed", this.g);
        dh2.a("abilities", dh3);
    }

    public void b(dh dh2) {
        if (dh2.b("abilities", 10)) {
            dh dh3 = dh2.m("abilities");
            this.a = dh3.n("invulnerable");
            this.b = dh3.n("flying");
            this.c = dh3.n("mayfly");
            this.d = dh3.n("instabuild");
            if (dh3.b("flySpeed", 99)) {
                this.f = dh3.h("flySpeed");
                this.g = dh3.h("walkSpeed");
            }
            if (dh3.b("mayBuild", 1)) {
                this.e = dh3.n("mayBuild");
            }
        }
    }

    public float a() {
        return this.f;
    }

    public float b() {
        return this.g;
    }
}

