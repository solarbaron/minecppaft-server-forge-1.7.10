/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.List;

public class gu
extends ft {
    private double a;
    private double b;
    private double c;
    private float d;
    private List e;
    private float f;
    private float g;
    private float h;

    public gu() {
    }

    public gu(double d2, double d3, double d4, float f2, List list, azw azw2) {
        this.a = d2;
        this.b = d3;
        this.c = d4;
        this.d = f2;
        this.e = new ArrayList(list);
        if (azw2 != null) {
            this.f = (float)azw2.a;
            this.g = (float)azw2.b;
            this.h = (float)azw2.c;
        }
    }

    @Override
    public void a(et et2) {
        this.a = et2.readFloat();
        this.b = et2.readFloat();
        this.c = et2.readFloat();
        this.d = et2.readFloat();
        int n2 = et2.readInt();
        this.e = new ArrayList(n2);
        int n3 = (int)this.a;
        int n4 = (int)this.b;
        int n5 = (int)this.c;
        for (int i2 = 0; i2 < n2; ++i2) {
            int n6 = et2.readByte() + n3;
            int n7 = et2.readByte() + n4;
            int n8 = et2.readByte() + n5;
            this.e.add(new agt(n6, n7, n8));
        }
        this.f = et2.readFloat();
        this.g = et2.readFloat();
        this.h = et2.readFloat();
    }

    @Override
    public void b(et et2) {
        et2.writeFloat((float)this.a);
        et2.writeFloat((float)this.b);
        et2.writeFloat((float)this.c);
        et2.writeFloat(this.d);
        et2.writeInt(this.e.size());
        int n2 = (int)this.a;
        int n3 = (int)this.b;
        int n4 = (int)this.c;
        for (agt agt2 : this.e) {
            int n5 = agt2.a - n2;
            int n6 = agt2.b - n3;
            int n7 = agt2.c - n4;
            et2.writeByte(n5);
            et2.writeByte(n6);
            et2.writeByte(n7);
        }
        et2.writeFloat(this.f);
        et2.writeFloat(this.g);
        et2.writeFloat(this.h);
    }

    public void a(fv fv2) {
        fv2.a(this);
    }
}

