/*
 * Decompiled with CFR 0.152.
 */
import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class gk
extends ft {
    private static final Logger a = LogManager.getLogger();
    private agu b;
    private byte[] c;
    private int d;

    public gk() {
    }

    public gk(int n2, short[] sArray, apx apx2) {
        this.b = new agu(apx2.g, apx2.h);
        this.d = n2;
        int n3 = 4 * n2;
        try {
            ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream(n3);
            DataOutputStream dataOutputStream = new DataOutputStream(byteArrayOutputStream);
            for (int i2 = 0; i2 < n2; ++i2) {
                int n4 = sArray[i2] >> 12 & 0xF;
                int n5 = sArray[i2] >> 8 & 0xF;
                int n6 = sArray[i2] & 0xFF;
                dataOutputStream.writeShort(sArray[i2]);
                dataOutputStream.writeShort((short)((aji.b(apx2.a(n4, n6, n5)) & 0xFFF) << 4 | apx2.c(n4, n6, n5) & 0xF));
            }
            this.c = byteArrayOutputStream.toByteArray();
            if (this.c.length != n3) {
                throw new RuntimeException("Expected length " + n3 + " doesn't match received length " + this.c.length);
            }
        }
        catch (IOException iOException) {
            a.error("Couldn't create bulk block update packet", (Throwable)iOException);
            this.c = null;
        }
    }

    @Override
    public void a(et et2) {
        this.b = new agu(et2.readInt(), et2.readInt());
        this.d = et2.readShort() & 0xFFFF;
        int n2 = et2.readInt();
        if (n2 > 0) {
            this.c = new byte[n2];
            et2.readBytes(this.c);
        }
    }

    @Override
    public void b(et et2) {
        et2.writeInt(this.b.a);
        et2.writeInt(this.b.b);
        et2.writeShort((short)this.d);
        if (this.c != null) {
            et2.writeInt(this.c.length);
            et2.writeBytes(this.c);
        } else {
            et2.writeInt(0);
        }
    }

    public void a(fv fv2) {
        fv2.a(this);
    }

    @Override
    public String b() {
        return String.format("xc=%d, zc=%d, count=%d", this.b.a, this.b.b, this.d);
    }
}

