/*
 * Decompiled with CFR 0.152.
 */
public class gv
extends ft {
    public static final String[] a = new String[]{"tile.bed.notValid", null, null, "gameMode.changed"};
    private int b;
    private float c;

    public gv() {
    }

    public gv(int n2, float f2) {
        this.b = n2;
        this.c = f2;
    }

    @Override
    public void a(et et2) {
        this.b = et2.readUnsignedByte();
        this.c = et2.readFloat();
    }

    @Override
    public void b(et et2) {
        et2.writeByte(this.b);
        et2.writeFloat(this.c);
    }

    public void a(fv fv2) {
        fv2.a(this);
    }
}

