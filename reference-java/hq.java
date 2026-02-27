/*
 * Decompiled with CFR 0.152.
 */
public class hq
extends ft {
    private int[] a;

    public hq() {
    }

    public hq(int ... nArray) {
        this.a = nArray;
    }

    @Override
    public void a(et et2) {
        this.a = new int[et2.readByte()];
        for (int i2 = 0; i2 < this.a.length; ++i2) {
            this.a[i2] = et2.readInt();
        }
    }

    @Override
    public void b(et et2) {
        et2.writeByte(this.a.length);
        for (int i2 = 0; i2 < this.a.length; ++i2) {
            et2.writeInt(this.a[i2]);
        }
    }

    public void a(fv fv2) {
        fv2.a(this);
    }

    @Override
    public String b() {
        StringBuilder stringBuilder = new StringBuilder();
        for (int i2 = 0; i2 < this.a.length; ++i2) {
            if (i2 > 0) {
                stringBuilder.append(", ");
            }
            stringBuilder.append(this.a[i2]);
        }
        return String.format("entities=%d[%s]", this.a.length, stringBuilder);
    }
}

