/*
 * Decompiled with CFR 0.152.
 */
public class jp
extends ft {
    private int a;
    private String b;
    private int c;
    private eo d;

    @Override
    public void a(et et2) {
        this.a = et2.a();
        this.b = et2.c(255);
        this.c = et2.readUnsignedShort();
        this.d = eo.a(et2.a());
    }

    @Override
    public void b(et et2) {
        et2.b(this.a);
        et2.a(this.b);
        et2.writeShort(this.c);
        et2.b(this.d.c());
    }

    public void a(jq jq2) {
        jq2.a(this);
    }

    @Override
    public boolean a() {
        return true;
    }

    public eo c() {
        return this.d;
    }

    public int d() {
        return this.a;
    }
}

