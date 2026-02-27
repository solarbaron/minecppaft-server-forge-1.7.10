/*
 * Decompiled with CFR 0.152.
 */
public class fy
extends ft {
    private int a;
    private int b;
    private int c;
    private int d;
    private int e;

    public fy() {
    }

    public fy(sa sa2) {
        this.a = sa2.y();
        this.b = qh.c(sa2.s * 32.0);
        this.c = qh.c(sa2.t * 32.0);
        this.d = qh.c(sa2.u * 32.0);
        if (sa2 instanceof xh) {
            this.e = 1;
        }
    }

    @Override
    public void a(et et2) {
        this.a = et2.a();
        this.e = et2.readByte();
        this.b = et2.readInt();
        this.c = et2.readInt();
        this.d = et2.readInt();
    }

    @Override
    public void b(et et2) {
        et2.b(this.a);
        et2.writeByte(this.e);
        et2.writeInt(this.b);
        et2.writeInt(this.c);
        et2.writeInt(this.d);
    }

    public void a(fv fv2) {
        fv2.a(this);
    }

    @Override
    public String b() {
        return String.format("id=%d, type=%d, x=%.2f, y=%.2f, z=%.2f", this.a, this.e, Float.valueOf((float)this.b / 32.0f), Float.valueOf((float)this.c / 32.0f), Float.valueOf((float)this.d / 32.0f));
    }
}

