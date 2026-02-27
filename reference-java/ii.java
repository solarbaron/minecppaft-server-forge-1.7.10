/*
 * Decompiled with CFR 0.152.
 */
public class ii
extends ft {
    private int a;
    private int b;
    private int c;
    private String[] d;

    public ii() {
    }

    public ii(int n2, int n3, int n4, String[] stringArray) {
        this.a = n2;
        this.b = n3;
        this.c = n4;
        this.d = new String[]{stringArray[0], stringArray[1], stringArray[2], stringArray[3]};
    }

    @Override
    public void a(et et2) {
        this.a = et2.readInt();
        this.b = et2.readShort();
        this.c = et2.readInt();
        this.d = new String[4];
        for (int i2 = 0; i2 < 4; ++i2) {
            this.d[i2] = et2.c(15);
        }
    }

    @Override
    public void b(et et2) {
        et2.writeInt(this.a);
        et2.writeShort(this.b);
        et2.writeInt(this.c);
        for (int i2 = 0; i2 < 4; ++i2) {
            et2.a(this.d[i2]);
        }
    }

    public void a(fv fv2) {
        fv2.a(this);
    }
}

