/*
 * Decompiled with CFR 0.152.
 */
import java.io.DataInput;
import java.io.DataOutput;
import java.util.Arrays;

public class dn
extends dy {
    private int[] b;

    dn() {
    }

    public dn(int[] nArray) {
        this.b = nArray;
    }

    @Override
    void a(DataOutput dataOutput) {
        dataOutput.writeInt(this.b.length);
        for (int i2 = 0; i2 < this.b.length; ++i2) {
            dataOutput.writeInt(this.b[i2]);
        }
    }

    @Override
    void a(DataInput dataInput, int n2, ds ds2) {
        int n3 = dataInput.readInt();
        ds2.a(32 * n3);
        this.b = new int[n3];
        for (int i2 = 0; i2 < n3; ++i2) {
            this.b[i2] = dataInput.readInt();
        }
    }

    @Override
    public byte a() {
        return 11;
    }

    @Override
    public String toString() {
        String string = "[";
        for (int n2 : this.b) {
            string = string + n2 + ",";
        }
        return string + "]";
    }

    @Override
    public dy b() {
        int[] nArray = new int[this.b.length];
        System.arraycopy(this.b, 0, nArray, 0, this.b.length);
        return new dn(nArray);
    }

    @Override
    public boolean equals(Object object) {
        if (super.equals(object)) {
            return Arrays.equals(this.b, ((dn)object).b);
        }
        return false;
    }

    @Override
    public int hashCode() {
        return super.hashCode() ^ Arrays.hashCode(this.b);
    }

    public int[] c() {
        return this.b;
    }
}

