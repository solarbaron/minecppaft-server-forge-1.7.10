/*
 * Decompiled with CFR 0.152.
 */
import java.io.IOException;
import java.util.zip.DataFormatException;
import java.util.zip.Deflater;
import java.util.zip.Inflater;

public class gx
extends ft {
    private int a;
    private int b;
    private int c;
    private int d;
    private byte[] e;
    private byte[] f;
    private boolean g;
    private int h;
    private static byte[] i = new byte[196864];

    public gx() {
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    public gx(apx apx2, boolean bl2, int n2) {
        this.a = apx2.g;
        this.b = apx2.h;
        this.g = bl2;
        gy gy2 = gx.a(apx2, bl2, n2);
        Deflater deflater = new Deflater(-1);
        this.d = gy2.c;
        this.c = gy2.b;
        try {
            this.f = gy2.a;
            deflater.setInput(gy2.a, 0, gy2.a.length);
            deflater.finish();
            this.e = new byte[gy2.a.length];
            this.h = deflater.deflate(this.e);
        }
        finally {
            deflater.end();
        }
    }

    public static int c() {
        return 196864;
    }

    @Override
    public void a(et et2) {
        int n2;
        this.a = et2.readInt();
        this.b = et2.readInt();
        this.g = et2.readBoolean();
        this.c = et2.readShort();
        this.d = et2.readShort();
        this.h = et2.readInt();
        if (i.length < this.h) {
            i = new byte[this.h];
        }
        et2.readBytes(i, 0, this.h);
        int n3 = 0;
        for (n2 = 0; n2 < 16; ++n2) {
            n3 += this.c >> n2 & 1;
        }
        n2 = 12288 * n3;
        if (this.g) {
            n2 += 256;
        }
        this.f = new byte[n2];
        Inflater inflater = new Inflater();
        inflater.setInput(i, 0, this.h);
        try {
            inflater.inflate(this.f);
        }
        catch (DataFormatException dataFormatException) {
            throw new IOException("Bad compressed data format");
        }
        finally {
            inflater.end();
        }
    }

    @Override
    public void b(et et2) {
        et2.writeInt(this.a);
        et2.writeInt(this.b);
        et2.writeBoolean(this.g);
        et2.writeShort((short)(this.c & 0xFFFF));
        et2.writeShort((short)(this.d & 0xFFFF));
        et2.writeInt(this.h);
        et2.writeBytes(this.e, 0, this.h);
    }

    public void a(fv fv2) {
        fv2.a(this);
    }

    @Override
    public String b() {
        return String.format("x=%d, z=%d, full=%b, sects=%d, add=%d, size=%d", this.a, this.b, this.g, this.c, this.d, this.h);
    }

    public static gy a(apx apx2, boolean bl2, int n2) {
        Object object;
        int n3;
        int n4 = 0;
        apz[] apzArray = apx2.i();
        int n5 = 0;
        gy gy2 = new gy();
        byte[] byArray = i;
        if (bl2) {
            apx2.q = true;
        }
        for (n3 = 0; n3 < apzArray.length; ++n3) {
            if (apzArray[n3] == null || bl2 && apzArray[n3].a() || (n2 & 1 << n3) == 0) continue;
            gy2.b |= 1 << n3;
            if (apzArray[n3].i() == null) continue;
            gy2.c |= 1 << n3;
            ++n5;
        }
        for (n3 = 0; n3 < apzArray.length; ++n3) {
            if (apzArray[n3] == null || bl2 && apzArray[n3].a() || (n2 & 1 << n3) == 0) continue;
            object = apzArray[n3].g();
            System.arraycopy(object, 0, byArray, n4, ((byte[])object).length);
            n4 += ((byte[])object).length;
        }
        for (n3 = 0; n3 < apzArray.length; ++n3) {
            if (apzArray[n3] == null || bl2 && apzArray[n3].a() || (n2 & 1 << n3) == 0) continue;
            object = apzArray[n3].j();
            System.arraycopy(object.a, 0, byArray, n4, object.a.length);
            n4 += object.a.length;
        }
        for (n3 = 0; n3 < apzArray.length; ++n3) {
            if (apzArray[n3] == null || bl2 && apzArray[n3].a() || (n2 & 1 << n3) == 0) continue;
            object = apzArray[n3].k();
            System.arraycopy(object.a, 0, byArray, n4, object.a.length);
            n4 += object.a.length;
        }
        if (!apx2.e.t.g) {
            for (n3 = 0; n3 < apzArray.length; ++n3) {
                if (apzArray[n3] == null || bl2 && apzArray[n3].a() || (n2 & 1 << n3) == 0) continue;
                object = apzArray[n3].l();
                System.arraycopy(object.a, 0, byArray, n4, object.a.length);
                n4 += object.a.length;
            }
        }
        if (n5 > 0) {
            for (n3 = 0; n3 < apzArray.length; ++n3) {
                if (apzArray[n3] == null || bl2 && apzArray[n3].a() || apzArray[n3].i() == null || (n2 & 1 << n3) == 0) continue;
                object = apzArray[n3].i();
                System.arraycopy(object.a, 0, byArray, n4, object.a.length);
                n4 += object.a.length;
            }
        }
        if (bl2) {
            byte[] byArray2 = apx2.m();
            System.arraycopy(byArray2, 0, byArray, n4, byArray2.length);
            n4 += byArray2.length;
        }
        gy2.a = new byte[n4];
        System.arraycopy(byArray, 0, gy2.a, 0, n4);
        return gy2;
    }
}

