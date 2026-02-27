/*
 * Decompiled with CFR 0.152.
 */
import java.io.DataInput;
import java.io.DataOutput;

public abstract class dy {
    public static final String[] a = new String[]{"END", "BYTE", "SHORT", "INT", "LONG", "FLOAT", "DOUBLE", "BYTE[]", "STRING", "LIST", "COMPOUND", "INT[]"};

    abstract void a(DataOutput var1);

    abstract void a(DataInput var1, int var2, ds var3);

    public abstract String toString();

    public abstract byte a();

    protected dy() {
    }

    protected static dy a(byte by2) {
        switch (by2) {
            case 0: {
                return new dl();
            }
            case 1: {
                return new dg();
            }
            case 2: {
                return new dw();
            }
            case 3: {
                return new dp();
            }
            case 4: {
                return new dr();
            }
            case 5: {
                return new dm();
            }
            case 6: {
                return new dk();
            }
            case 7: {
                return new df();
            }
            case 11: {
                return new dn();
            }
            case 8: {
                return new dx();
            }
            case 9: {
                return new dq();
            }
            case 10: {
                return new dh();
            }
        }
        return null;
    }

    public abstract dy b();

    public boolean equals(Object object) {
        if (!(object instanceof dy)) {
            return false;
        }
        dy dy2 = (dy)object;
        return this.a() == dy2.a();
    }

    public int hashCode() {
        return this.a();
    }

    protected String a_() {
        return this.toString();
    }
}

