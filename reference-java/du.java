/*
 * Decompiled with CFR 0.152.
 */
import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInput;
import java.io.DataInputStream;
import java.io.DataOutput;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.zip.GZIPInputStream;
import java.util.zip.GZIPOutputStream;

public class du {
    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    public static dh a(InputStream inputStream) {
        DataInputStream dataInputStream = new DataInputStream(new BufferedInputStream(new GZIPInputStream(inputStream)));
        try {
            dh dh2 = du.a(dataInputStream, ds.a);
            return dh2;
        }
        finally {
            dataInputStream.close();
        }
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    public static void a(dh dh2, OutputStream outputStream) {
        DataOutputStream dataOutputStream = new DataOutputStream(new BufferedOutputStream(new GZIPOutputStream(outputStream)));
        try {
            du.a(dh2, (DataOutput)dataOutputStream);
        }
        finally {
            dataOutputStream.close();
        }
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    public static dh a(byte[] byArray, ds ds2) {
        DataInputStream dataInputStream = new DataInputStream(new BufferedInputStream(new GZIPInputStream(new ByteArrayInputStream(byArray))));
        try {
            dh dh2 = du.a(dataInputStream, ds2);
            return dh2;
        }
        finally {
            dataInputStream.close();
        }
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    public static byte[] a(dh dh2) {
        ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();
        DataOutputStream dataOutputStream = new DataOutputStream(new GZIPOutputStream(byteArrayOutputStream));
        try {
            du.a(dh2, (DataOutput)dataOutputStream);
        }
        finally {
            dataOutputStream.close();
        }
        return byteArrayOutputStream.toByteArray();
    }

    public static dh a(DataInputStream dataInputStream) {
        return du.a(dataInputStream, ds.a);
    }

    public static dh a(DataInput dataInput, ds ds2) {
        dy dy2 = du.a(dataInput, 0, ds2);
        if (dy2 instanceof dh) {
            return (dh)dy2;
        }
        throw new IOException("Root tag must be a named compound tag");
    }

    public static void a(dh dh2, DataOutput dataOutput) {
        du.a((dy)dh2, dataOutput);
    }

    private static void a(dy dy2, DataOutput dataOutput) {
        dataOutput.writeByte(dy2.a());
        if (dy2.a() == 0) {
            return;
        }
        dataOutput.writeUTF("");
        dy2.a(dataOutput);
    }

    private static dy a(DataInput dataInput, int n2, ds ds2) {
        byte by2 = dataInput.readByte();
        if (by2 == 0) {
            return new dl();
        }
        dataInput.readUTF();
        dy dy2 = dy.a(by2);
        try {
            dy2.a(dataInput, n2, ds2);
        }
        catch (IOException iOException) {
            b b2 = b.a(iOException, "Loading NBT data");
            k k2 = b2.a("NBT Tag");
            k2.a("Tag name", "[UNNAMED TAG]");
            k2.a("Tag type", by2);
            throw new s(b2);
        }
        return dy2;
    }
}

