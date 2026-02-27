/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;

public class go
extends ft {
    private int a;
    private add[] b;

    public go() {
    }

    public go(int n2, List list) {
        this.a = n2;
        this.b = new add[list.size()];
        for (int i2 = 0; i2 < this.b.length; ++i2) {
            add add2 = (add)list.get(i2);
            this.b[i2] = add2 == null ? null : add2.m();
        }
    }

    @Override
    public void a(et et2) {
        this.a = et2.readUnsignedByte();
        int n2 = et2.readShort();
        this.b = new add[n2];
        for (int i2 = 0; i2 < n2; ++i2) {
            this.b[i2] = et2.c();
        }
    }

    @Override
    public void b(et et2) {
        et2.writeByte(this.a);
        et2.writeShort(this.b.length);
        for (add add2 : this.b) {
            et2.a(add2);
        }
    }

    public void a(fv fv2) {
        fv2.a(this);
    }
}

