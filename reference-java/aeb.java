/*
 * Decompiled with CFR 0.152.
 */
import com.mojang.authlib.GameProfile;

public class aeb
extends adb {
    private static final String[] b = new String[]{"skeleton", "wither", "zombie", "char", "creeper"};
    public static final String[] a = new String[]{"skeleton", "wither", "zombie", "steve", "creeper"};

    public aeb() {
        this.a(abt.c);
        this.f(0);
        this.a(true);
    }

    @Override
    public boolean a(add add2, yz yz2, ahb ahb2, int n2, int n3, int n4, int n5, float f2, float f3, float f4) {
        if (n5 == 0) {
            return false;
        }
        if (!ahb2.a(n2, n3, n4).o().a()) {
            return false;
        }
        if (n5 == 1) {
            ++n3;
        }
        if (n5 == 2) {
            --n4;
        }
        if (n5 == 3) {
            ++n4;
        }
        if (n5 == 4) {
            --n2;
        }
        if (n5 == 5) {
            ++n2;
        }
        if (!ahb2.E) {
            aor aor2;
            ahb2.d(n2, n3, n4, ajn.bP, n5, 2);
            int n6 = 0;
            if (n5 == 1) {
                n6 = qh.c((double)(yz2.y * 16.0f / 360.0f) + 0.5) & 0xF;
            }
            if ((aor2 = ahb2.o(n2, n3, n4)) != null && aor2 instanceof apn) {
                if (add2.k() == 3) {
                    GameProfile gameProfile = null;
                    if (add2.p()) {
                        dh dh2 = add2.q();
                        if (dh2.b("SkullOwner", 10)) {
                            gameProfile = dv.a(dh2.m("SkullOwner"));
                        } else if (dh2.b("SkullOwner", 8) && dh2.j("SkullOwner").length() > 0) {
                            gameProfile = new GameProfile(null, dh2.j("SkullOwner"));
                        }
                    }
                    ((apn)aor2).a(gameProfile);
                } else {
                    ((apn)aor2).a(add2.k());
                }
                ((apn)aor2).b(n6);
                ((anl)ajn.bP).a(ahb2, n2, n3, n4, (apn)aor2);
            }
            --add2.b;
        }
        return true;
    }

    @Override
    public int a(int n2) {
        return n2;
    }

    @Override
    public String a(add add2) {
        int n2 = add2.k();
        if (n2 < 0 || n2 >= b.length) {
            n2 = 0;
        }
        return super.a() + "." + b[n2];
    }

    @Override
    public String n(add add2) {
        if (add2.k() == 3 && add2.p()) {
            if (add2.q().b("SkullOwner", 10)) {
                return dd.a("item.skull.player.name", dv.a(add2.q().m("SkullOwner")).getName());
            }
            if (add2.q().b("SkullOwner", 8)) {
                return dd.a("item.skull.player.name", add2.q().j("SkullOwner"));
            }
        }
        return super.n(add2);
    }
}

