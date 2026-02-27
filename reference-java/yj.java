/*
 * Decompiled with CFR 0.152.
 */
import java.util.Collection;
import java.util.UUID;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class yj {
    private static final Logger f = LogManager.getLogger();
    public static final th a = new to("generic.maxHealth", 20.0, 0.0, Double.MAX_VALUE).a("Max Health").a(true);
    public static final th b = new to("generic.followRange", 32.0, 0.0, 2048.0).a("Follow Range");
    public static final th c = new to("generic.knockbackResistance", 0.0, 0.0, 1.0).a("Knockback Resistance");
    public static final th d = new to("generic.movementSpeed", 0.7f, 0.0, Double.MAX_VALUE).a("Movement Speed").a(true);
    public static final th e = new to("generic.attackDamage", 2.0, 0.0, Double.MAX_VALUE);

    public static dq a(tl tl2) {
        dq dq2 = new dq();
        for (ti ti2 : tl2.a()) {
            dq2.a(yj.a(ti2));
        }
        return dq2;
    }

    private static dh a(ti ti2) {
        dh dh2 = new dh();
        th th2 = ti2.a();
        dh2.a("Name", th2.a());
        dh2.a("Base", ti2.b());
        Collection collection = ti2.c();
        if (collection != null && !collection.isEmpty()) {
            dq dq2 = new dq();
            for (tj tj2 : collection) {
                if (!tj2.e()) continue;
                dq2.a(yj.a(tj2));
            }
            dh2.a("Modifiers", dq2);
        }
        return dh2;
    }

    private static dh a(tj tj2) {
        dh dh2 = new dh();
        dh2.a("Name", tj2.b());
        dh2.a("Amount", tj2.d());
        dh2.a("Operation", tj2.c());
        dh2.a("UUIDMost", tj2.a().getMostSignificantBits());
        dh2.a("UUIDLeast", tj2.a().getLeastSignificantBits());
        return dh2;
    }

    public static void a(tl tl2, dq dq2) {
        for (int i2 = 0; i2 < dq2.c(); ++i2) {
            dh dh2 = dq2.b(i2);
            ti ti2 = tl2.a(dh2.j("Name"));
            if (ti2 != null) {
                yj.a(ti2, dh2);
                continue;
            }
            f.warn("Ignoring unknown attribute '" + dh2.j("Name") + "'");
        }
    }

    private static void a(ti ti2, dh dh2) {
        ti2.a(dh2.i("Base"));
        if (dh2.b("Modifiers", 9)) {
            dq dq2 = dh2.c("Modifiers", 10);
            for (int i2 = 0; i2 < dq2.c(); ++i2) {
                tj tj2 = yj.a(dq2.b(i2));
                tj tj3 = ti2.a(tj2.a());
                if (tj3 != null) {
                    ti2.b(tj3);
                }
                ti2.a(tj2);
            }
        }
    }

    public static tj a(dh dh2) {
        UUID uUID = new UUID(dh2.g("UUIDMost"), dh2.g("UUIDLeast"));
        return new tj(uUID, dh2.j("Name"), dh2.i("Amount"), dh2.f("Operation"));
    }
}

