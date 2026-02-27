/*
 * Decompiled with CFR 0.152.
 */
import org.apache.commons.lang3.Validate;

public class hc
extends ft {
    private String a;
    private int b;
    private int c = Integer.MAX_VALUE;
    private int d;
    private float e;
    private int f;

    public hc() {
    }

    public hc(String string, double d2, double d3, double d4, float f2, float f3) {
        Validate.notNull(string, "name", new Object[0]);
        this.a = string;
        this.b = (int)(d2 * 8.0);
        this.c = (int)(d3 * 8.0);
        this.d = (int)(d4 * 8.0);
        this.e = f2;
        this.f = (int)(f3 * 63.0f);
        if (this.f < 0) {
            this.f = 0;
        }
        if (this.f > 255) {
            this.f = 255;
        }
    }

    @Override
    public void a(et et2) {
        this.a = et2.c(256);
        this.b = et2.readInt();
        this.c = et2.readInt();
        this.d = et2.readInt();
        this.e = et2.readFloat();
        this.f = et2.readUnsignedByte();
    }

    @Override
    public void b(et et2) {
        et2.a(this.a);
        et2.writeInt(this.b);
        et2.writeInt(this.c);
        et2.writeInt(this.d);
        et2.writeFloat(this.e);
        et2.writeByte(this.f);
    }

    public void a(fv fv2) {
        fv2.a(this);
    }
}

