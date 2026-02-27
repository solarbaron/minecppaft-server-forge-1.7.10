/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;

public class pp {
    protected static Map a = new HashMap();
    public static List b = new ArrayList();
    public static List c = new ArrayList();
    public static List d = new ArrayList();
    public static List e = new ArrayList();
    public static ph f = new pe("stat.leaveGame", new fr("stat.leaveGame", new Object[0])).i().h();
    public static ph g = new pe("stat.playOneMinute", new fr("stat.playOneMinute", new Object[0]), ph.h).i().h();
    public static ph h = new pe("stat.walkOneCm", new fr("stat.walkOneCm", new Object[0]), ph.i).i().h();
    public static ph i = new pe("stat.swimOneCm", new fr("stat.swimOneCm", new Object[0]), ph.i).i().h();
    public static ph j = new pe("stat.fallOneCm", new fr("stat.fallOneCm", new Object[0]), ph.i).i().h();
    public static ph k = new pe("stat.climbOneCm", new fr("stat.climbOneCm", new Object[0]), ph.i).i().h();
    public static ph l = new pe("stat.flyOneCm", new fr("stat.flyOneCm", new Object[0]), ph.i).i().h();
    public static ph m = new pe("stat.diveOneCm", new fr("stat.diveOneCm", new Object[0]), ph.i).i().h();
    public static ph n = new pe("stat.minecartOneCm", new fr("stat.minecartOneCm", new Object[0]), ph.i).i().h();
    public static ph o = new pe("stat.boatOneCm", new fr("stat.boatOneCm", new Object[0]), ph.i).i().h();
    public static ph p = new pe("stat.pigOneCm", new fr("stat.pigOneCm", new Object[0]), ph.i).i().h();
    public static ph q = new pe("stat.horseOneCm", new fr("stat.horseOneCm", new Object[0]), ph.i).i().h();
    public static ph r = new pe("stat.jump", new fr("stat.jump", new Object[0])).i().h();
    public static ph s = new pe("stat.drop", new fr("stat.drop", new Object[0])).i().h();
    public static ph t = new pe("stat.damageDealt", new fr("stat.damageDealt", new Object[0]), ph.j).h();
    public static ph u = new pe("stat.damageTaken", new fr("stat.damageTaken", new Object[0]), ph.j).h();
    public static ph v = new pe("stat.deaths", new fr("stat.deaths", new Object[0])).h();
    public static ph w = new pe("stat.mobKills", new fr("stat.mobKills", new Object[0])).h();
    public static ph x = new pe("stat.animalsBred", new fr("stat.animalsBred", new Object[0])).h();
    public static ph y = new pe("stat.playerKills", new fr("stat.playerKills", new Object[0])).h();
    public static ph z = new pe("stat.fishCaught", new fr("stat.fishCaught", new Object[0])).h();
    public static ph A = new pe("stat.junkFished", new fr("stat.junkFished", new Object[0])).h();
    public static ph B = new pe("stat.treasureFished", new fr("stat.treasureFished", new Object[0])).h();
    public static final ph[] C = new ph[4096];
    public static final ph[] D = new ph[32000];
    public static final ph[] E = new ph[32000];
    public static final ph[] F = new ph[32000];

    public static void a() {
        pp.c();
        pp.d();
        pp.e();
        pp.b();
        pc.a();
        sg.a();
    }

    private static void b() {
        HashSet<adb> hashSet = new HashSet<adb>();
        for (afg object : afe.a().b()) {
            if (object.b() == null) continue;
            hashSet.add(object.b().b());
        }
        for (add add2 : afa.a().b().values()) {
            hashSet.add(add2.b());
        }
        for (adb adb2 : hashSet) {
            if (adb2 == null) continue;
            int n2 = adb.b(adb2);
            pp.D[n2] = new pf("stat.craftItem." + n2, (fj)new fr("stat.craftItem", new add(adb2).E()), adb2).h();
        }
        pp.a(D);
    }

    private static void c() {
        for (aji aji2 : aji.c) {
            if (adb.a(aji2) == null) continue;
            int n2 = aji.b(aji2);
            if (!aji2.G()) continue;
            pp.C[n2] = new pf("stat.mineBlock." + n2, (fj)new fr("stat.mineBlock", new add(aji2).E()), adb.a(aji2)).h();
            e.add((pf)C[n2]);
        }
        pp.a(C);
    }

    private static void d() {
        for (adb adb2 : adb.e) {
            if (adb2 == null) continue;
            int n2 = adb.b(adb2);
            pp.E[n2] = new pf("stat.useItem." + n2, (fj)new fr("stat.useItem", new add(adb2).E()), adb2).h();
            if (adb2 instanceof abh) continue;
            d.add((pf)E[n2]);
        }
        pp.a(E);
    }

    private static void e() {
        for (adb adb2 : adb.e) {
            if (adb2 == null) continue;
            int n2 = adb.b(adb2);
            if (!adb2.p()) continue;
            pp.F[n2] = new pf("stat.breakItem." + n2, (fj)new fr("stat.breakItem", new add(adb2).E()), adb2).h();
        }
        pp.a(F);
    }

    private static void a(ph[] phArray) {
        pp.a(phArray, ajn.j, ajn.i);
        pp.a(phArray, ajn.l, ajn.k);
        pp.a(phArray, ajn.aP, ajn.aK);
        pp.a(phArray, ajn.am, ajn.al);
        pp.a(phArray, ajn.ay, ajn.ax);
        pp.a(phArray, ajn.aS, ajn.aR);
        pp.a(phArray, ajn.bV, ajn.bU);
        pp.a(phArray, ajn.aA, ajn.az);
        pp.a(phArray, ajn.bv, ajn.bu);
        pp.a(phArray, ajn.Q, ajn.P);
        pp.a(phArray, ajn.T, ajn.U);
        pp.a(phArray, ajn.bw, ajn.bx);
        pp.a(phArray, ajn.c, ajn.d);
        pp.a(phArray, ajn.ak, ajn.d);
    }

    private static void a(ph[] phArray, aji aji2, aji aji3) {
        int n2 = aji.b(aji2);
        int n3 = aji.b(aji3);
        if (phArray[n2] != null && phArray[n3] == null) {
            phArray[n3] = phArray[n2];
            return;
        }
        b.remove(phArray[n2]);
        e.remove(phArray[n2]);
        c.remove(phArray[n2]);
        phArray[n2] = phArray[n3];
    }

    public static ph a(sh sh2) {
        String string = sg.b(sh2.a);
        if (string == null) {
            return null;
        }
        return new ph("stat.killEntity." + string, new fr("stat.entityKill", new fr("entity." + string + ".name", new Object[0]))).h();
    }

    public static ph b(sh sh2) {
        String string = sg.b(sh2.a);
        if (string == null) {
            return null;
        }
        return new ph("stat.entityKilledBy." + string, new fr("stat.entityKilledBy", new fr("entity." + string + ".name", new Object[0]))).h();
    }

    public static ph a(String string) {
        return (ph)a.get(string);
    }
}

