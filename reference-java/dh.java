/*
 * Decompiled with CFR 0.152.
 */
import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class dh
extends dy {
    private static final Logger b = LogManager.getLogger();
    private Map c = new HashMap();

    @Override
    void a(DataOutput dataOutput) {
        for (String string : this.c.keySet()) {
            dy dy2 = (dy)this.c.get(string);
            dh.a(string, dy2, dataOutput);
        }
        dataOutput.writeByte(0);
    }

    @Override
    void a(DataInput dataInput, int n2, ds ds2) {
        byte by2;
        if (n2 > 512) {
            throw new RuntimeException("Tried to read NBT tag with too high complexity, depth > 512");
        }
        this.c.clear();
        while ((by2 = dh.a(dataInput, ds2)) != 0) {
            String string = dh.b(dataInput, ds2);
            ds2.a(16 * string.length());
            dy dy2 = dh.a(by2, string, dataInput, n2 + 1, ds2);
            this.c.put(string, dy2);
        }
    }

    public Set c() {
        return this.c.keySet();
    }

    @Override
    public byte a() {
        return 10;
    }

    public void a(String string, dy dy2) {
        this.c.put(string, dy2);
    }

    public void a(String string, byte by2) {
        this.c.put(string, new dg(by2));
    }

    public void a(String string, short s2) {
        this.c.put(string, new dw(s2));
    }

    public void a(String string, int n2) {
        this.c.put(string, new dp(n2));
    }

    public void a(String string, long l2) {
        this.c.put(string, new dr(l2));
    }

    public void a(String string, float f2) {
        this.c.put(string, new dm(f2));
    }

    public void a(String string, double d2) {
        this.c.put(string, new dk(d2));
    }

    public void a(String string, String string2) {
        this.c.put(string, new dx(string2));
    }

    public void a(String string, byte[] byArray) {
        this.c.put(string, new df(byArray));
    }

    public void a(String string, int[] nArray) {
        this.c.put(string, new dn(nArray));
    }

    public void a(String string, boolean bl2) {
        this.a(string, bl2 ? (byte)1 : 0);
    }

    public dy a(String string) {
        return (dy)this.c.get(string);
    }

    public byte b(String string) {
        dy dy2 = (dy)this.c.get(string);
        if (dy2 != null) {
            return dy2.a();
        }
        return 0;
    }

    public boolean c(String string) {
        return this.c.containsKey(string);
    }

    public boolean b(String string, int n2) {
        byte by2 = this.b(string);
        if (by2 == n2) {
            return true;
        }
        if (n2 == 99) {
            return by2 == 1 || by2 == 2 || by2 == 3 || by2 == 4 || by2 == 5 || by2 == 6;
        }
        return false;
    }

    public byte d(String string) {
        try {
            if (!this.c.containsKey(string)) {
                return 0;
            }
            return ((dz)this.c.get(string)).f();
        }
        catch (ClassCastException classCastException) {
            return 0;
        }
    }

    public short e(String string) {
        try {
            if (!this.c.containsKey(string)) {
                return 0;
            }
            return ((dz)this.c.get(string)).e();
        }
        catch (ClassCastException classCastException) {
            return 0;
        }
    }

    public int f(String string) {
        try {
            if (!this.c.containsKey(string)) {
                return 0;
            }
            return ((dz)this.c.get(string)).d();
        }
        catch (ClassCastException classCastException) {
            return 0;
        }
    }

    public long g(String string) {
        try {
            if (!this.c.containsKey(string)) {
                return 0L;
            }
            return ((dz)this.c.get(string)).c();
        }
        catch (ClassCastException classCastException) {
            return 0L;
        }
    }

    public float h(String string) {
        try {
            if (!this.c.containsKey(string)) {
                return 0.0f;
            }
            return ((dz)this.c.get(string)).h();
        }
        catch (ClassCastException classCastException) {
            return 0.0f;
        }
    }

    public double i(String string) {
        try {
            if (!this.c.containsKey(string)) {
                return 0.0;
            }
            return ((dz)this.c.get(string)).g();
        }
        catch (ClassCastException classCastException) {
            return 0.0;
        }
    }

    public String j(String string) {
        try {
            if (!this.c.containsKey(string)) {
                return "";
            }
            return ((dy)this.c.get(string)).a_();
        }
        catch (ClassCastException classCastException) {
            return "";
        }
    }

    public byte[] k(String string) {
        try {
            if (!this.c.containsKey(string)) {
                return new byte[0];
            }
            return ((df)this.c.get(string)).c();
        }
        catch (ClassCastException classCastException) {
            throw new s(this.a(string, 7, classCastException));
        }
    }

    public int[] l(String string) {
        try {
            if (!this.c.containsKey(string)) {
                return new int[0];
            }
            return ((dn)this.c.get(string)).c();
        }
        catch (ClassCastException classCastException) {
            throw new s(this.a(string, 11, classCastException));
        }
    }

    public dh m(String string) {
        try {
            if (!this.c.containsKey(string)) {
                return new dh();
            }
            return (dh)this.c.get(string);
        }
        catch (ClassCastException classCastException) {
            throw new s(this.a(string, 10, classCastException));
        }
    }

    public dq c(String string, int n2) {
        try {
            if (this.b(string) != 9) {
                return new dq();
            }
            dq dq2 = (dq)this.c.get(string);
            if (dq2.c() > 0 && dq2.d() != n2) {
                return new dq();
            }
            return dq2;
        }
        catch (ClassCastException classCastException) {
            throw new s(this.a(string, 9, classCastException));
        }
    }

    public boolean n(String string) {
        return this.d(string) != 0;
    }

    public void o(String string) {
        this.c.remove(string);
    }

    @Override
    public String toString() {
        String string = "{";
        for (String string2 : this.c.keySet()) {
            string = string + string2 + ':' + this.c.get(string2) + ',';
        }
        return string + "}";
    }

    public boolean d() {
        return this.c.isEmpty();
    }

    private b a(String string, int n2, ClassCastException classCastException) {
        b b2 = b.a(classCastException, "Reading NBT data");
        k k2 = b2.a("Corrupt NBT tag", 1);
        k2.a("Tag type found", new di(this, string));
        k2.a("Tag type expected", new dj(this, n2));
        k2.a("Tag name", string);
        return b2;
    }

    @Override
    public dy b() {
        dh dh2 = new dh();
        for (String string : this.c.keySet()) {
            dh2.a(string, ((dy)this.c.get(string)).b());
        }
        return dh2;
    }

    @Override
    public boolean equals(Object object) {
        if (super.equals(object)) {
            dh dh2 = (dh)object;
            return this.c.entrySet().equals(dh2.c.entrySet());
        }
        return false;
    }

    @Override
    public int hashCode() {
        return super.hashCode() ^ this.c.hashCode();
    }

    private static void a(String string, dy dy2, DataOutput dataOutput) {
        dataOutput.writeByte(dy2.a());
        if (dy2.a() == 0) {
            return;
        }
        dataOutput.writeUTF(string);
        dy2.a(dataOutput);
    }

    private static byte a(DataInput dataInput, ds ds2) {
        return dataInput.readByte();
    }

    private static String b(DataInput dataInput, ds ds2) {
        return dataInput.readUTF();
    }

    static dy a(byte by2, String string, DataInput dataInput, int n2, ds ds2) {
        dy dy2 = dy.a(by2);
        try {
            dy2.a(dataInput, n2, ds2);
        }
        catch (IOException iOException) {
            b b2 = b.a(iOException, "Loading NBT data");
            k k2 = b2.a("NBT Tag");
            k2.a("Tag name", string);
            k2.a("Tag type", by2);
            throw new s(b2);
        }
        return dy2;
    }

    static /* synthetic */ Map a(dh dh2) {
        return dh2.c;
    }
}

