/*
 * Decompiled with CFR 0.152.
 */
public class abb
extends adb {
    private static final int[] m = new int[]{11, 16, 15, 13};
    private static final String[] n = new String[]{"leather_helmet_overlay", "leather_chestplate_overlay", "leather_leggings_overlay", "leather_boots_overlay"};
    public static final String[] a = new String[]{"empty_armor_slot_helmet", "empty_armor_slot_chestplate", "empty_armor_slot_leggings", "empty_armor_slot_boots"};
    private static final cp o = new abc();
    public final int b;
    public final int c;
    public final int d;
    private final abd p;

    public abb(abd abd2, int n2, int n3) {
        this.p = abd2;
        this.b = n3;
        this.d = n2;
        this.c = abd2.b(n3);
        this.f(abd2.a(n3));
        this.h = 1;
        this.a(abt.j);
        akm.a.a(this, o);
    }

    @Override
    public int c() {
        return this.p.a();
    }

    public abd m_() {
        return this.p;
    }

    public boolean c_(add add2) {
        if (this.p != abd.a) {
            return false;
        }
        if (!add2.p()) {
            return false;
        }
        if (!add2.q().b("display", 10)) {
            return false;
        }
        return add2.q().m("display").b("color", 3);
    }

    public int b(add add2) {
        if (this.p != abd.a) {
            return -1;
        }
        dh dh2 = add2.q();
        if (dh2 == null) {
            return 10511680;
        }
        dh dh3 = dh2.m("display");
        if (dh3 == null) {
            return 10511680;
        }
        if (dh3.b("color", 3)) {
            return dh3.f("color");
        }
        return 10511680;
    }

    public void c(add add2) {
        if (this.p != abd.a) {
            return;
        }
        dh dh2 = add2.q();
        if (dh2 == null) {
            return;
        }
        dh dh3 = dh2.m("display");
        if (dh3.c("color")) {
            dh3.o("color");
        }
    }

    public void b(add add2, int n2) {
        if (this.p != abd.a) {
            throw new UnsupportedOperationException("Can't dye non-leather!");
        }
        dh dh2 = add2.q();
        if (dh2 == null) {
            dh2 = new dh();
            add2.d(dh2);
        }
        dh dh3 = dh2.m("display");
        if (!dh2.b("display", 10)) {
            dh2.a("display", dh3);
        }
        dh3.a("color", n2);
    }

    @Override
    public boolean a(add add2, add add3) {
        if (this.p.b() == add3.b()) {
            return true;
        }
        return super.a(add2, add3);
    }

    @Override
    public add a(add add2, ahb ahb2, yz yz2) {
        int n2 = sw.b(add2) - 1;
        add add3 = yz2.r(n2);
        if (add3 == null) {
            yz2.c(n2, add2.m());
            add2.b = 0;
        }
        return add2;
    }

    static /* synthetic */ int[] e() {
        return m;
    }
}

