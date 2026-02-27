/*
 * Decompiled with CFR 0.152.
 */
import java.io.DataInput;
import java.io.DataOutput;

public class dk
extends dz {
    private double b;

    dk() {
    }

    public dk(double d2) {
        this.b = d2;
    }

    @Override
    void a(DataOutput dataOutput) {
        dataOutput.writeDouble(this.b);
    }

    @Override
    void a(DataInput dataInput, int n2, ds ds2) {
        ds2.a(64L);
        this.b = dataInput.readDouble();
    }

    @Override
    public byte a() {
        return 6;
    }

    @Override
    public String toString() {
        return "" + this.b + "d";
    }

    @Override
    public dy b() {
        return new dk(this.b);
    }

    @Override
    public boolean equals(Object object) {
        if (super.equals(object)) {
            dk dk2 = (dk)object;
            return this.b == dk2.b;
        }
        return false;
    }

    @Override
    public int hashCode() {
        long l2 = Double.doubleToLongBits(this.b);
        return super.hashCode() ^ (int)(l2 ^ l2 >>> 32);
    }

    @Override
    public long c() {
        return (long)Math.floor(this.b);
    }

    @Override
    public int d() {
        return qh.c(this.b);
    }

    @Override
    public short e() {
        return (short)(qh.c(this.b) & 0xFFFF);
    }

    @Override
    public byte f() {
        return (byte)(qh.c(this.b) & 0xFF);
    }

    @Override
    public double g() {
        return this.b;
    }

    @Override
    public float h() {
        return (float)this.b;
    }
}

