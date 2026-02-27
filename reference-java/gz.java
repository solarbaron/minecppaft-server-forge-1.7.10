/*
 * Decompiled with CFR 0.152.
 */
import java.io.IOException;
import java.util.List;
import java.util.zip.DataFormatException;
import java.util.zip.Deflater;
import java.util.zip.Inflater;

public class gz
extends ft {
    private int[] a;
    private int[] b;
    private int[] c;
    private int[] d;
    private byte[] e;
    private byte[][] f;
    private int g;
    private boolean h;
    private static byte[] i = new byte[0];

    public gz() {
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    public gz(List list) {
        int n2 = list.size();
        this.a = new int[n2];
        this.b = new int[n2];
        this.c = new int[n2];
        this.d = new int[n2];
        this.f = new byte[n2][];
        this.h = !list.isEmpty() && !((apx)list.get((int)0)).e.t.g;
        int n3 = 0;
        for (int i2 = 0; i2 < n2; ++i2) {
            apx apx2 = (apx)list.get(i2);
            gy gy2 = gx.a(apx2, true, 65535);
            if (i.length < n3 + gy2.a.length) {
                byte[] byArray = new byte[n3 + gy2.a.length];
                System.arraycopy(i, 0, byArray, 0, i.length);
                i = byArray;
            }
            System.arraycopy(gy2.a, 0, i, n3, gy2.a.length);
            n3 += gy2.a.length;
            this.a[i2] = apx2.g;
            this.b[i2] = apx2.h;
            this.c[i2] = gy2.b;
            this.d[i2] = gy2.c;
            this.f[i2] = gy2.a;
        }
        Deflater deflater = new Deflater(-1);
        try {
            deflater.setInput(i, 0, n3);
            deflater.finish();
            this.e = new byte[n3];
            this.g = deflater.deflate(this.e);
        }
        finally {
            deflater.end();
        }
    }

    public static int c() {
        return 5;
    }

    @Override
    public void a(et et2) {
        int n2 = et2.readShort();
        this.g = et2.readInt();
        this.h = et2.readBoolean();
        this.a = new int[n2];
        this.b = new int[n2];
        this.c = new int[n2];
        this.d = new int[n2];
        this.f = new byte[n2][];
        if (i.length < this.g) {
            i = new byte[this.g];
        }
        et2.readBytes(i, 0, this.g);
        byte[] byArray = new byte[gx.c() * n2];
        Inflater inflater = new Inflater();
        inflater.setInput(i, 0, this.g);
        try {
            inflater.inflate(byArray);
        }
        catch (DataFormatException dataFormatException) {
            throw new IOException("Bad compressed data format");
        }
        finally {
            inflater.end();
        }
        int n3 = 0;
        for (int i2 = 0; i2 < n2; ++i2) {
            int n4;
            this.a[i2] = et2.readInt();
            this.b[i2] = et2.readInt();
            this.c[i2] = et2.readShort();
            this.d[i2] = et2.readShort();
            int n5 = 0;
            int n6 = 0;
            for (n4 = 0; n4 < 16; ++n4) {
                n5 += this.c[i2] >> n4 & 1;
                n6 += this.d[i2] >> n4 & 1;
            }
            n4 = 2048 * (4 * n5) + 256;
            n4 += 2048 * n6;
            if (this.h) {
                n4 += 2048 * n5;
            }
            this.f[i2] = new byte[n4];
            System.arraycopy(byArray, n3, this.f[i2], 0, n4);
            n3 += n4;
        }
    }

    @Override
    public void b(et et2) {
        et2.writeShort(this.a.length);
        et2.writeInt(this.g);
        et2.writeBoolean(this.h);
        et2.writeBytes(this.e, 0, this.g);
        for (int i2 = 0; i2 < this.a.length; ++i2) {
            et2.writeInt(this.a[i2]);
            et2.writeInt(this.b[i2]);
            et2.writeShort((short)(this.c[i2] & 0xFFFF));
            et2.writeShort((short)(this.d[i2] & 0xFFFF));
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
            stringBuilder.append(String.format("{x=%d, z=%d, sections=%d, adds=%d, data=%d}", this.a[i2], this.b[i2], this.c[i2], this.d[i2], this.f[i2].length));
        }
        return String.format("size=%d, chunks=%d[%s]", this.g, this.a.length, stringBuilder);
    }
}

