/*
 * Decompiled with CFR 0.152.
 */
public class fw
extends ft {
    private int a;
    private int b;
    private int c;
    private int d;
    private int e;
    private int f;
    private int g;
    private int h;
    private int i;
    private int j;
    private int k;

    public fw() {
    }

    public fw(sa sa2, int n2) {
        this(sa2, n2, 0);
    }

    public fw(sa sa2, int n2, int n3) {
        this.a = sa2.y();
        this.b = qh.c(sa2.s * 32.0);
        this.c = qh.c(sa2.t * 32.0);
        this.d = qh.c(sa2.u * 32.0);
        this.h = qh.d(sa2.z * 256.0f / 360.0f);
        this.i = qh.d(sa2.y * 256.0f / 360.0f);
        this.j = n2;
        this.k = n3;
        if (n3 > 0) {
            double d2 = sa2.v;
            double d3 = sa2.w;
            double d4 = sa2.x;
            double d5 = 3.9;
            if (d2 < -d5) {
                d2 = -d5;
            }
            if (d3 < -d5) {
                d3 = -d5;
            }
            if (d4 < -d5) {
                d4 = -d5;
            }
            if (d2 > d5) {
                d2 = d5;
            }
            if (d3 > d5) {
                d3 = d5;
            }
            if (d4 > d5) {
                d4 = d5;
            }
            this.e = (int)(d2 * 8000.0);
            this.f = (int)(d3 * 8000.0);
            this.g = (int)(d4 * 8000.0);
        }
    }

    @Override
    public void a(et et2) {
        this.a = et2.a();
        this.j = et2.readByte();
        this.b = et2.readInt();
        this.c = et2.readInt();
        this.d = et2.readInt();
        this.h = et2.readByte();
        this.i = et2.readByte();
        this.k = et2.readInt();
        if (this.k > 0) {
            this.e = et2.readShort();
            this.f = et2.readShort();
            this.g = et2.readShort();
        }
    }

    @Override
    public void b(et et2) {
        et2.b(this.a);
        et2.writeByte(this.j);
        et2.writeInt(this.b);
        et2.writeInt(this.c);
        et2.writeInt(this.d);
        et2.writeByte(this.h);
        et2.writeByte(this.i);
        et2.writeInt(this.k);
        if (this.k > 0) {
            et2.writeShort(this.e);
            et2.writeShort(this.f);
            et2.writeShort(this.g);
        }
    }

    public void a(fv fv2) {
        fv2.a(this);
    }

    @Override
    public String b() {
        return String.format("id=%d, type=%d, x=%.2f, y=%.2f, z=%.2f", this.a, this.j, Float.valueOf((float)this.b / 32.0f), Float.valueOf((float)this.c / 32.0f), Float.valueOf((float)this.d / 32.0f));
    }

    public void a(int n2) {
        this.b = n2;
    }

    public void b(int n2) {
        this.c = n2;
    }

    public void c(int n2) {
        this.d = n2;
    }

    public void d(int n2) {
        this.e = n2;
    }

    public void e(int n2) {
        this.f = n2;
    }

    public void f(int n2) {
        this.g = n2;
    }
}

