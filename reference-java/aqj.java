/*
 * Decompiled with CFR 0.152.
 */
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

public class aqj {
    private static final Map a = new HashMap();

    public static synchronized aqh a(File file, int n2, int n3) {
        File file2 = new File(file, "region");
        File file3 = new File(file2, "r." + (n2 >> 5) + "." + (n3 >> 5) + ".mca");
        aqh aqh2 = (aqh)a.get(file3);
        if (aqh2 != null) {
            return aqh2;
        }
        if (!file2.exists()) {
            file2.mkdirs();
        }
        if (a.size() >= 256) {
            aqj.a();
        }
        aqh aqh3 = new aqh(file3);
        a.put(file3, aqh3);
        return aqh3;
    }

    public static synchronized void a() {
        for (aqh aqh2 : a.values()) {
            try {
                if (aqh2 == null) continue;
                aqh2.c();
            }
            catch (IOException iOException) {
                iOException.printStackTrace();
            }
        }
        a.clear();
    }

    public static DataInputStream c(File file, int n2, int n3) {
        aqh aqh2 = aqj.a(file, n2, n3);
        return aqh2.a(n2 & 0x1F, n3 & 0x1F);
    }

    public static DataOutputStream d(File file, int n2, int n3) {
        aqh aqh2 = aqj.a(file, n2, n3);
        return aqh2.b(n2 & 0x1F, n3 & 0x1F);
    }
}

