/*
 * Decompiled with CFR 0.152.
 */
import java.util.HashMap;
import java.util.Map;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class aor {
    private static final Logger a = LogManager.getLogger();
    private static Map i = new HashMap();
    private static Map j = new HashMap();
    protected ahb b;
    public int c;
    public int d;
    public int e;
    protected boolean f;
    public int g = -1;
    public aji h;

    private static void a(Class clazz, String string) {
        if (i.containsKey(string)) {
            throw new IllegalArgumentException("Duplicate id: " + string);
        }
        i.put(string, clazz);
        j.put(clazz, string);
    }

    public ahb w() {
        return this.b;
    }

    public void a(ahb ahb2) {
        this.b = ahb2;
    }

    public boolean o() {
        return this.b != null;
    }

    public void a(dh dh2) {
        this.c = dh2.f("x");
        this.d = dh2.f("y");
        this.e = dh2.f("z");
    }

    public void b(dh dh2) {
        String string = (String)j.get(this.getClass());
        if (string == null) {
            throw new RuntimeException(this.getClass() + " is missing a mapping! This is a bug!");
        }
        dh2.a("id", string);
        dh2.a("x", this.c);
        dh2.a("y", this.d);
        dh2.a("z", this.e);
    }

    public void h() {
    }

    public static aor c(dh dh2) {
        aor aor2 = null;
        try {
            Class clazz = (Class)i.get(dh2.j("id"));
            if (clazz != null) {
                aor2 = (aor)clazz.newInstance();
            }
        }
        catch (Exception exception) {
            exception.printStackTrace();
        }
        if (aor2 != null) {
            aor2.a(dh2);
        } else {
            a.warn("Skipping BlockEntity with id " + dh2.j("id"));
        }
        return aor2;
    }

    public int p() {
        if (this.g == -1) {
            this.g = this.b.e(this.c, this.d, this.e);
        }
        return this.g;
    }

    public void e() {
        if (this.b != null) {
            this.g = this.b.e(this.c, this.d, this.e);
            this.b.b(this.c, this.d, this.e, this);
            if (this.q() != ajn.a) {
                this.b.f(this.c, this.d, this.e, this.q());
            }
        }
    }

    public aji q() {
        if (this.h == null) {
            this.h = this.b.a(this.c, this.d, this.e);
        }
        return this.h;
    }

    public ft m() {
        return null;
    }

    public boolean r() {
        return this.f;
    }

    public void s() {
        this.f = true;
    }

    public void t() {
        this.f = false;
    }

    public boolean c(int n2, int n3) {
        return false;
    }

    public void u() {
        this.h = null;
        this.g = -1;
    }

    public void a(k k2) {
        k2.a("Name", new aos(this));
        k.a(k2, this.c, this.d, this.e, this.q(), this.p());
        k2.a("Actual block type", new aot(this));
        k2.a("Actual block data value", new aou(this));
    }

    static /* synthetic */ Map v() {
        return j;
    }

    static {
        aor.a(apg.class, "Furnace");
        aor.a(aow.class, "Chest");
        aor.a(ape.class, "EnderChest");
        aor.a(alr.class, "RecordPlayer");
        aor.a(apb.class, "Trap");
        aor.a(apc.class, "Dropper");
        aor.a(apm.class, "Sign");
        aor.a(apj.class, "MobSpawner");
        aor.a(apl.class, "Music");
        aor.a(aps.class, "Piston");
        aor.a(aov.class, "Cauldron");
        aor.a(apd.class, "EnchantTable");
        aor.a(apo.class, "Airportal");
        aor.a(aox.class, "Control");
        aor.a(aoq.class, "Beacon");
        aor.a(apn.class, "Skull");
        aor.a(apa.class, "DLDetector");
        aor.a(api.class, "Hopper");
        aor.a(aoz.class, "Comparator");
        aor.a(apf.class, "FlowerPot");
    }
}

