/*
 * Decompiled with CFR 0.152.
 */
import java.io.DataInput;
import java.io.DataOutput;
import java.util.ArrayList;
import java.util.List;

public class dq
extends dy {
    private List b = new ArrayList();
    private byte c = 0;

    @Override
    void a(DataOutput dataOutput) {
        this.c = !this.b.isEmpty() ? ((dy)this.b.get(0)).a() : (byte)0;
        dataOutput.writeByte(this.c);
        dataOutput.writeInt(this.b.size());
        for (int i2 = 0; i2 < this.b.size(); ++i2) {
            ((dy)this.b.get(i2)).a(dataOutput);
        }
    }

    @Override
    void a(DataInput dataInput, int n2, ds ds2) {
        if (n2 > 512) {
            throw new RuntimeException("Tried to read NBT tag with too high complexity, depth > 512");
        }
        ds2.a(8L);
        this.c = dataInput.readByte();
        int n3 = dataInput.readInt();
        this.b = new ArrayList();
        for (int i2 = 0; i2 < n3; ++i2) {
            dy dy2 = dy.a(this.c);
            dy2.a(dataInput, n2 + 1, ds2);
            this.b.add(dy2);
        }
    }

    @Override
    public byte a() {
        return 9;
    }

    @Override
    public String toString() {
        String string = "[";
        int n2 = 0;
        for (dy dy2 : this.b) {
            string = string + "" + n2 + ':' + dy2 + ',';
            ++n2;
        }
        return string + "]";
    }

    public void a(dy dy2) {
        if (this.c == 0) {
            this.c = dy2.a();
        } else if (this.c != dy2.a()) {
            System.err.println("WARNING: Adding mismatching tag types to tag list");
            return;
        }
        this.b.add(dy2);
    }

    public dh b(int n2) {
        if (n2 < 0 || n2 >= this.b.size()) {
            return new dh();
        }
        dy dy2 = (dy)this.b.get(n2);
        if (dy2.a() == 10) {
            return (dh)dy2;
        }
        return new dh();
    }

    public int[] c(int n2) {
        if (n2 < 0 || n2 >= this.b.size()) {
            return new int[0];
        }
        dy dy2 = (dy)this.b.get(n2);
        if (dy2.a() == 11) {
            return ((dn)dy2).c();
        }
        return new int[0];
    }

    public double d(int n2) {
        if (n2 < 0 || n2 >= this.b.size()) {
            return 0.0;
        }
        dy dy2 = (dy)this.b.get(n2);
        if (dy2.a() == 6) {
            return ((dk)dy2).g();
        }
        return 0.0;
    }

    public float e(int n2) {
        if (n2 < 0 || n2 >= this.b.size()) {
            return 0.0f;
        }
        dy dy2 = (dy)this.b.get(n2);
        if (dy2.a() == 5) {
            return ((dm)dy2).h();
        }
        return 0.0f;
    }

    public String f(int n2) {
        if (n2 < 0 || n2 >= this.b.size()) {
            return "";
        }
        dy dy2 = (dy)this.b.get(n2);
        if (dy2.a() == 8) {
            return dy2.a_();
        }
        return dy2.toString();
    }

    public int c() {
        return this.b.size();
    }

    @Override
    public dy b() {
        dq dq2 = new dq();
        dq2.c = this.c;
        for (dy dy2 : this.b) {
            dy dy3 = dy2.b();
            dq2.b.add(dy3);
        }
        return dq2;
    }

    @Override
    public boolean equals(Object object) {
        if (super.equals(object)) {
            dq dq2 = (dq)object;
            if (this.c == dq2.c) {
                return this.b.equals(dq2.b);
            }
        }
        return false;
    }

    @Override
    public int hashCode() {
        return super.hashCode() ^ this.b.hashCode();
    }

    public int d() {
        return this.c;
    }
}

