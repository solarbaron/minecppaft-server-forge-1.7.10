/*
 * Decompiled with CFR 0.152.
 */
import java.util.Collections;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Set;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class sg {
    private static final Logger b = LogManager.getLogger();
    private static Map c = new HashMap();
    private static Map d = new HashMap();
    private static Map e = new HashMap();
    private static Map f = new HashMap();
    private static Map g = new HashMap();
    public static HashMap a = new LinkedHashMap();

    private static void a(Class clazz, String string, int n2) {
        if (c.containsKey(string)) {
            throw new IllegalArgumentException("ID is already registered: " + string);
        }
        if (e.containsKey(n2)) {
            throw new IllegalArgumentException("ID is already registered: " + n2);
        }
        c.put(string, clazz);
        d.put(clazz, string);
        e.put(n2, clazz);
        f.put(clazz, n2);
        g.put(string, n2);
    }

    private static void a(Class clazz, String string, int n2, int n3, int n4) {
        sg.a(clazz, string, n2);
        a.put(n2, new sh(n2, n3, n4));
    }

    public static sa a(String string, ahb ahb2) {
        sa sa2 = null;
        try {
            Class clazz = (Class)c.get(string);
            if (clazz != null) {
                sa2 = (sa)clazz.getConstructor(ahb.class).newInstance(ahb2);
            }
        }
        catch (Exception exception) {
            exception.printStackTrace();
        }
        return sa2;
    }

    public static sa a(dh dh2, ahb ahb2) {
        sa sa2 = null;
        if ("Minecart".equals(dh2.j("id"))) {
            switch (dh2.f("Type")) {
                case 1: {
                    dh2.a("id", "MinecartChest");
                    break;
                }
                case 2: {
                    dh2.a("id", "MinecartFurnace");
                    break;
                }
                case 0: {
                    dh2.a("id", "MinecartRideable");
                }
            }
            dh2.o("Type");
        }
        try {
            Class clazz = (Class)c.get(dh2.j("id"));
            if (clazz != null) {
                sa2 = (sa)clazz.getConstructor(ahb.class).newInstance(ahb2);
            }
        }
        catch (Exception exception) {
            exception.printStackTrace();
        }
        if (sa2 != null) {
            sa2.f(dh2);
        } else {
            b.warn("Skipping Entity with id " + dh2.j("id"));
        }
        return sa2;
    }

    public static sa a(int n2, ahb ahb2) {
        sa sa2 = null;
        try {
            Class clazz = sg.a(n2);
            if (clazz != null) {
                sa2 = (sa)clazz.getConstructor(ahb.class).newInstance(ahb2);
            }
        }
        catch (Exception exception) {
            exception.printStackTrace();
        }
        if (sa2 == null) {
            b.warn("Skipping Entity with id " + n2);
        }
        return sa2;
    }

    public static int a(sa sa2) {
        Class<?> clazz = sa2.getClass();
        return f.containsKey(clazz) ? (Integer)f.get(clazz) : 0;
    }

    public static Class a(int n2) {
        return (Class)e.get(n2);
    }

    public static String b(sa sa2) {
        return (String)d.get(sa2.getClass());
    }

    public static String b(int n2) {
        Class clazz = sg.a(n2);
        if (clazz != null) {
            return (String)d.get(clazz);
        }
        return null;
    }

    public static void a() {
    }

    public static Set b() {
        return Collections.unmodifiableSet(g.keySet());
    }

    static {
        sg.a(xk.class, "Item", 1);
        sg.a(sq.class, "XPOrb", 2);
        sg.a(su.class, "LeashKnot", 8);
        sg.a(tb.class, "Painting", 9);
        sg.a(zc.class, "Arrow", 10);
        sg.a(zj.class, "Snowball", 11);
        sg.a(zg.class, "Fireball", 12);
        sg.a(zi.class, "SmallFireball", 13);
        sg.a(zm.class, "ThrownEnderpearl", 14);
        sg.a(zd.class, "EyeOfEnderSignal", 15);
        sg.a(zo.class, "ThrownPotion", 16);
        sg.a(zn.class, "ThrownExpBottle", 17);
        sg.a(st.class, "ItemFrame", 18);
        sg.a(zp.class, "WitherSkull", 19);
        sg.a(xw.class, "PrimedTnt", 20);
        sg.a(xj.class, "FallingSand", 21);
        sg.a(zf.class, "FireworksRocketEntity", 22);
        sg.a(xi.class, "Boat", 41);
        sg.a(xs.class, "MinecartRideable", 42);
        sg.a(xm.class, "MinecartChest", 43);
        sg.a(xq.class, "MinecartFurnace", 44);
        sg.a(xv.class, "MinecartTNT", 45);
        sg.a(xr.class, "MinecartHopper", 46);
        sg.a(xt.class, "MinecartSpawner", 47);
        sg.a(xn.class, "MinecartCommandBlock", 40);
        sg.a(sw.class, "Mob", 48);
        sg.a(yg.class, "Monster", 49);
        sg.a(xz.class, "Creeper", 50, 894731, 0);
        sg.a(yl.class, "Skeleton", 51, 0xC1C1C1, 0x494949);
        sg.a(yn.class, "Spider", 52, 3419431, 11013646);
        sg.a(ye.class, "Giant", 53);
        sg.a(yq.class, "Zombie", 54, 44975, 7969893);
        sg.a(ym.class, "Slime", 55, 5349438, 8306542);
        sg.a(yd.class, "Ghast", 56, 0xF9F9F9, 0xBCBCBC);
        sg.a(yh.class, "PigZombie", 57, 15373203, 5009705);
        sg.a(ya.class, "Enderman", 58, 0x161616, 0);
        sg.a(xy.class, "CaveSpider", 59, 803406, 11013646);
        sg.a(yk.class, "Silverfish", 60, 0x6E6E6E, 0x303030);
        sg.a(xx.class, "Blaze", 61, 16167425, 16775294);
        sg.a(yf.class, "LavaSlime", 62, 0x340000, 0xFCFC00);
        sg.a(xa.class, "EnderDragon", 63);
        sg.a(xc.class, "WitherBoss", 64);
        sg.a(we.class, "Bat", 65, 4996656, 986895);
        sg.a(yp.class, "Witch", 66, 0x340000, 5349438);
        sg.a(wo.class, "Pig", 90, 15771042, 14377823);
        sg.a(wp.class, "Sheep", 91, 0xE7E7E7, 0xFFB5B5);
        sg.a(wh.class, "Cow", 92, 4470310, 0xA1A1A1);
        sg.a(wg.class, "Chicken", 93, 0xA1A1A1, 0xFF0000);
        sg.a(ws.class, "Squid", 94, 2243405, 7375001);
        sg.a(wv.class, "Wolf", 95, 0xD7D3D3, 13545366);
        sg.a(wm.class, "MushroomCow", 96, 10489616, 0xB7B7B7);
        sg.a(wr.class, "SnowMan", 97);
        sg.a(wn.class, "Ozelot", 98, 15720061, 5653556);
        sg.a(wt.class, "VillagerGolem", 99);
        sg.a(wi.class, "EntityHorse", 100, 12623485, 0xEEE500);
        sg.a(yv.class, "Villager", 120, 5651507, 12422002);
        sg.a(wz.class, "EnderCrystal", 200);
    }
}

