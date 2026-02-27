/*
 * Decompiled with CFR 0.152.
 */
public class hy
extends ft {
    private int a;
    private int b;
    private int c;
    private int d;

    public hy() {
    }

    public hy(sa sa2) {
        this(sa2.y(), sa2.v, sa2.w, sa2.x);
    }

    public hy(int n2, double d2, double d3, double d4) {
        this.a = n2;
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
        this.b = (int)(d2 * 8000.0);
        this.c = (int)(d3 * 8000.0);
        this.d = (int)(d4 * 8000.0);
    }

    @Override
    public void a(et et2) {
        this.a = et2.readInt();
        this.b = et2.readShort();
        this.c = et2.readShort();
        this.d = et2.readShort();
    }

    @Override
    public void b(et et2) {
        et2.writeInt(this.a);
        et2.writeShort(this.b);
        et2.writeShort(this.c);
        et2.writeShort(this.d);
    }

    public void a(fv fv2) {
        fv2.a(this);
    }

    @Override
    public String b() {
        return String.format("id=%d, x=%.2f, y=%.2f, z=%.2f", this.a, Float.valueOf((float)this.b / 8000.0f), Float.valueOf((float)this.c / 8000.0f), Float.valueOf((float)this.d / 8000.0f));
    }
}

