/*
 * Decompiled with CFR 0.152.
 */
import java.io.DataInput;
import java.io.DataOutput;

public class dp
extends dz {
    private int b;

    dp() {
    }

    public dp(int n2) {
        this.b = n2;
    }

    @Override
    void a(DataOutput dataOutput) {
        dataOutput.writeInt(this.b);
    }

    @Override
    void a(DataInput dataInput, int n2, ds ds2) {
        ds2.a(32L);
        this.b = dataInput.readInt();
    }

    @Override
    public byte a() {
        return 3;
    }

    @Override
    public String toString() {
        return "" + this.b;
    }

    @Override
    public dy b() {
        return new dp(this.b);
    }

    @Override
    public boolean equals(Object object) {
        if (super.equals(object)) {
            dp dp2 = (dp)object;
            return this.b == dp2.b;
        }
        return false;
    }

    @Override
    public int hashCode() {
        return super.hashCode() ^ this.b;
    }

    @Override
    public long c() {
        return this.b;
    }

    @Override
    public int d() {
        return this.b;
    }

    @Override
    public short e() {
        return (short)(this.b & 0xFFFF);
    }

    @Override
    public byte f() {
        return (byte)(this.b & 0xFF);
    }

    @Override
    public double g() {
        return this.b;
    }

    @Override
    public float h() {
        return this.b;
    }
}

