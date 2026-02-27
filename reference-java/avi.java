/*
 * Decompiled with CFR 0.152.
 */
import java.util.HashMap;
import java.util.Map;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class avi {
    private static final Logger a = LogManager.getLogger();
    private static Map b = new HashMap();
    private static Map c = new HashMap();
    private static Map d = new HashMap();
    private static Map e = new HashMap();

    private static void b(Class clazz, String string) {
        b.put(string, clazz);
        c.put(clazz, string);
    }

    static void a(Class clazz, String string) {
        d.put(string, clazz);
        e.put(clazz, string);
    }

    public static String a(avm avm2) {
        return (String)c.get(avm2.getClass());
    }

    public static String a(avk avk2) {
        return (String)e.get(avk2.getClass());
    }

    public static avm a(dh dh2, ahb ahb2) {
        avm avm2 = null;
        try {
            Class clazz = (Class)b.get(dh2.j("id"));
            if (clazz != null) {
                avm2 = (avm)clazz.newInstance();
            }
        }
        catch (Exception exception) {
            a.warn("Failed Start with id " + dh2.j("id"));
            exception.printStackTrace();
        }
        if (avm2 != null) {
            avm2.a(ahb2, dh2);
        } else {
            a.warn("Skipping Structure with id " + dh2.j("id"));
        }
        return avm2;
    }

    public static avk b(dh dh2, ahb ahb2) {
        avk avk2 = null;
        try {
            Class clazz = (Class)d.get(dh2.j("id"));
            if (clazz != null) {
                avk2 = (avk)clazz.newInstance();
            }
        }
        catch (Exception exception) {
            a.warn("Failed Piece with id " + dh2.j("id"));
            exception.printStackTrace();
        }
        if (avk2 != null) {
            avk2.a(ahb2, dh2);
        } else {
            a.warn("Skipping Piece with id " + dh2.j("id"));
        }
        return avk2;
    }

    static {
        avi.b(atc.class, "Mineshaft");
        avi.b(avo.class, "Village");
        avi.b(ate.class, "Fortress");
        avi.b(auh.class, "Stronghold");
        avi.b(aty.class, "Temple");
        asx.a();
        avp.a();
        atf.a();
        aui.a();
        atz.a();
    }
}

