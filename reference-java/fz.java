/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;

public class fz
extends ft {
    private int a;
    private int b;
    private int c;
    private int d;
    private int e;
    private int f;
    private int g;
    private int h;
    private byte i;
    private byte j;
    private byte k;
    private te l;
    private List m;

    public fz() {
    }

    public fz(sv sv2) {
        this.a = sv2.y();
        this.b = (byte)sg.a(sv2);
        this.c = sv2.as.a(sv2.s);
        this.d = qh.c(sv2.t * 32.0);
        this.e = sv2.as.a(sv2.u);
        this.i = (byte)(sv2.y * 256.0f / 360.0f);
        this.j = (byte)(sv2.z * 256.0f / 360.0f);
        this.k = (byte)(sv2.aO * 256.0f / 360.0f);
        double d2 = 3.9;
        double d3 = sv2.v;
        double d4 = sv2.w;
        double d5 = sv2.x;
        if (d3 < -d2) {
            d3 = -d2;
        }
        if (d4 < -d2) {
            d4 = -d2;
        }
        if (d5 < -d2) {
            d5 = -d2;
        }
        if (d3 > d2) {
            d3 = d2;
        }
        if (d4 > d2) {
            d4 = d2;
        }
        if (d5 > d2) {
            d5 = d2;
        }
        this.f = (int)(d3 * 8000.0);
        this.g = (int)(d4 * 8000.0);
        this.h = (int)(d5 * 8000.0);
        this.l = sv2.z();
    }

    @Override
    public void a(et et2) {
        this.a = et2.a();
        this.b = et2.readByte() & 0xFF;
        this.c = et2.readInt();
        this.d = et2.readInt();
        this.e = et2.readInt();
        this.i = et2.readByte();
        this.j = et2.readByte();
        this.k = et2.readByte();
        this.f = et2.readShort();
        this.g = et2.readShort();
        this.h = et2.readShort();
        this.m = te.b(et2);
    }

    @Override
    public void b(et et2) {
        et2.b(this.a);
        et2.writeByte(this.b & 0xFF);
        et2.writeInt(this.c);
        et2.writeInt(this.d);
        et2.writeInt(this.e);
        et2.writeByte(this.i);
        et2.writeByte(this.j);
        et2.writeByte(this.k);
        et2.writeShort(this.f);
        et2.writeShort(this.g);
        et2.writeShort(this.h);
        this.l.a(et2);
    }

    public void a(fv fv2) {
        fv2.a(this);
    }

    @Override
    public String b() {
        return String.format("id=%d, type=%d, x=%.2f, y=%.2f, z=%.2f, xd=%.2f, yd=%.2f, zd=%.2f", this.a, this.b, Float.valueOf((float)this.c / 32.0f), Float.valueOf((float)this.d / 32.0f), Float.valueOf((float)this.e / 32.0f), Float.valueOf((float)this.f / 8000.0f), Float.valueOf((float)this.g / 8000.0f), Float.valueOf((float)this.h / 8000.0f));
    }
}

