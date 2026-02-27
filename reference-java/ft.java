/*
 * Decompiled with CFR 0.152.
 */
import com.google.common.collect.BiMap;
import io.netty.buffer.ByteBuf;
import java.io.IOException;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public abstract class ft {
    private static final Logger a = LogManager.getLogger();

    public static ft a(BiMap biMap, int n2) {
        try {
            Class clazz = (Class)biMap.get(n2);
            if (clazz == null) {
                return null;
            }
            return (ft)clazz.newInstance();
        }
        catch (Exception exception) {
            a.error("Couldn't create packet " + n2, (Throwable)exception);
            return null;
        }
    }

    public static void a(ByteBuf byteBuf, byte[] byArray) {
        byteBuf.writeShort(byArray.length);
        byteBuf.writeBytes(byArray);
    }

    public static byte[] a(ByteBuf byteBuf) {
        short s2 = byteBuf.readShort();
        if (s2 < 0) {
            throw new IOException("Key was smaller than nothing!  Weird key!");
        }
        byte[] byArray = new byte[s2];
        byteBuf.readBytes(byArray);
        return byArray;
    }

    public abstract void a(et var1);

    public abstract void b(et var1);

    public abstract void a(fb var1);

    public boolean a() {
        return false;
    }

    public String toString() {
        return this.getClass().getSimpleName();
    }

    public String b() {
        return "";
    }
}

