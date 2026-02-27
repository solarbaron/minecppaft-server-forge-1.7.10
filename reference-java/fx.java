/*
 * Decompiled with CFR 0.152.
 */
public class fx
extends ft {
    private int a;
    private int b;
    private int c;
    private int d;
    private int e;

    public fx() {
    }

    public fx(sq sq2) {
        this.a = sq2.y();
        this.b = qh.c(sq2.s * 32.0);
        this.c = qh.c(sq2.t * 32.0);
        this.d = qh.c(sq2.u * 32.0);
        this.e = sq2.e();
    }

    @Override
    public void a(et et2) {
        this.a = et2.a();
        this.b = et2.readInt();
        this.c = et2.readInt();
        this.d = et2.readInt();
        this.e = et2.readShort();
    }

    @Override
    public void b(et et2) {
        et2.b(this.a);
        et2.writeInt(this.b);
        et2.writeInt(this.c);
        et2.writeInt(this.d);
        et2.writeShort(this.e);
    }

    public void a(fv fv2) {
        fv2.a(this);
    }

    @Override
    public String b() {
        return String.format("id=%d, value=%d, x=%.2f, y=%.2f, z=%.2f", this.a, this.e, Float.valueOf((float)this.b / 32.0f), Float.valueOf((float)this.c / 32.0f), Float.valueOf((float)this.d / 32.0f));
    }
}

