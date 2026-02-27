/*
 * Decompiled with CFR 0.152.
 */
import com.google.common.collect.BiMap;
import com.google.common.collect.HashBiMap;
import com.google.common.collect.Iterables;
import com.google.common.collect.Maps;
import gnu.trove.map.TIntObjectMap;
import gnu.trove.map.hash.TIntObjectHashMap;
import java.util.Map;
import org.apache.logging.log4j.LogManager;

public class eo
extends Enum {
    public static final /* enum */ eo a = new ep(-1);
    public static final /* enum */ eo b = new eq(0);
    public static final /* enum */ eo c = new er(1);
    public static final /* enum */ eo d = new es(2);
    private static final TIntObjectMap e;
    private static final Map f;
    private final int g;
    private final BiMap h = HashBiMap.create();
    private final BiMap i = HashBiMap.create();
    private static final /* synthetic */ eo[] j;

    public static eo[] values() {
        return (eo[])j.clone();
    }

    public static eo valueOf(String string) {
        return Enum.valueOf(eo.class, string);
    }

    /*
     * WARNING - Possible parameter corruption
     * WARNING - void declaration
     */
    private eo() {
        void var3_2;
        void var2_-1;
        void var1_-1;
        this.g = var3_2;
    }

    protected eo a(int n2, Class clazz) {
        if (this.h.containsKey(n2)) {
            String string = "Serverbound packet ID " + n2 + " is already assigned to " + this.h.get(n2) + "; cannot re-assign to " + clazz;
            LogManager.getLogger().fatal(string);
            throw new IllegalArgumentException(string);
        }
        if (this.h.containsValue(clazz)) {
            String string = "Serverbound packet " + clazz + " is already assigned to ID " + this.h.inverse().get(clazz) + "; cannot re-assign to " + n2;
            LogManager.getLogger().fatal(string);
            throw new IllegalArgumentException(string);
        }
        this.h.put(n2, clazz);
        return this;
    }

    protected eo b(int n2, Class clazz) {
        if (this.i.containsKey(n2)) {
            String string = "Clientbound packet ID " + n2 + " is already assigned to " + this.i.get(n2) + "; cannot re-assign to " + clazz;
            LogManager.getLogger().fatal(string);
            throw new IllegalArgumentException(string);
        }
        if (this.i.containsValue(clazz)) {
            String string = "Clientbound packet " + clazz + " is already assigned to ID " + this.i.inverse().get(clazz) + "; cannot re-assign to " + n2;
            LogManager.getLogger().fatal(string);
            throw new IllegalArgumentException(string);
        }
        this.i.put(n2, clazz);
        return this;
    }

    public BiMap a() {
        return this.h;
    }

    public BiMap b() {
        return this.i;
    }

    public BiMap a(boolean bl2) {
        return bl2 ? this.b() : this.a();
    }

    public BiMap b(boolean bl2) {
        return bl2 ? this.a() : this.b();
    }

    public int c() {
        return this.g;
    }

    public static eo a(int n2) {
        return (eo)((Object)e.get(n2));
    }

    public static eo a(ft ft2) {
        return (eo)((Object)f.get(ft2.getClass()));
    }

    /*
     * WARNING - void declaration
     */
    /* synthetic */ eo(int n2, ep ep2) {
        this((String)var1_-1, (int)ep2, (int)var3_2);
        void var3_2;
        void var1_-1;
    }

    static {
        j = new eo[]{a, b, c, d};
        e = new TIntObjectHashMap();
        f = Maps.newHashMap();
        for (eo eo2 : eo.values()) {
            e.put(eo2.c(), eo2);
            for (Class clazz : Iterables.concat(eo2.b().values(), eo2.a().values())) {
                if (f.containsKey(clazz) && f.get(clazz) != eo2) {
                    throw new Error("Packet " + clazz + " is already assigned to protocol " + f.get(clazz) + " - can't reassign to " + (Object)((Object)eo2));
                }
                f.put(clazz, eo2);
            }
        }
    }
}

