/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.List;

public class pc {
    public static int a;
    public static int b;
    public static int c;
    public static int d;
    public static List e;
    public static pb f;
    public static pb g;
    public static pb h;
    public static pb i;
    public static pb j;
    public static pb k;
    public static pb l;
    public static pb m;
    public static pb n;
    public static pb o;
    public static pb p;
    public static pb q;
    public static pb r;
    public static pb s;
    public static pb t;
    public static pb u;
    public static pb v;
    public static pb w;
    public static pb x;
    public static pb y;
    public static pb z;
    public static pb A;
    public static pb B;
    public static pb C;
    public static pb D;
    public static pb E;
    public static pb F;
    public static pb G;
    public static pb H;
    public static pb I;
    public static pb J;
    public static pb K;
    public static pb L;

    public static void a() {
    }

    static {
        e = new ArrayList();
        f = new pb("achievement.openInventory", "openInventory", 0, 0, ade.aG, null).a().c();
        g = new pb("achievement.mineWood", "mineWood", 2, 1, ajn.r, f).c();
        h = new pb("achievement.buildWorkBench", "buildWorkBench", 4, -1, ajn.ai, g).c();
        i = new pb("achievement.buildPickaxe", "buildPickaxe", 4, 2, ade.o, h).c();
        j = new pb("achievement.buildFurnace", "buildFurnace", 3, 4, ajn.al, i).c();
        k = new pb("achievement.acquireIron", "acquireIron", 1, 4, ade.j, j).c();
        l = new pb("achievement.buildHoe", "buildHoe", 2, -3, ade.I, h).c();
        m = new pb("achievement.makeBread", "makeBread", -1, -3, ade.P, l).c();
        n = new pb("achievement.bakeCake", "bakeCake", 0, -5, ade.aU, l).c();
        o = new pb("achievement.buildBetterPickaxe", "buildBetterPickaxe", 6, 2, ade.s, i).c();
        p = new pb("achievement.cookFish", "cookFish", 2, 6, ade.aQ, j).c();
        q = new pb("achievement.onARail", "onARail", 2, 3, ajn.aq, k).b().c();
        r = new pb("achievement.buildSword", "buildSword", 6, -1, ade.m, h).c();
        s = new pb("achievement.killEnemy", "killEnemy", 8, -1, ade.aS, r).c();
        t = new pb("achievement.killCow", "killCow", 7, -3, ade.aA, r).c();
        u = new pb("achievement.flyPig", "flyPig", 9, -3, ade.av, t).b().c();
        v = new pb("achievement.snipeSkeleton", "snipeSkeleton", 7, 0, ade.f, s).b().c();
        w = new pb("achievement.diamonds", "diamonds", -1, 5, ajn.ag, k).c();
        x = new pb("achievement.diamondsToYou", "diamondsToYou", -1, 2, ade.i, w).c();
        y = new pb("achievement.portal", "portal", -1, 7, ajn.Z, w).c();
        z = new pb("achievement.ghast", "ghast", -4, 8, ade.bk, y).b().c();
        A = new pb("achievement.blazeRod", "blazeRod", 0, 9, ade.bj, y).c();
        B = new pb("achievement.potion", "potion", 2, 8, ade.bn, A).c();
        C = new pb("achievement.theEnd", "theEnd", 3, 10, ade.bv, A).b().c();
        D = new pb("achievement.theEnd2", "theEnd2", 4, 13, ajn.bt, C).b().c();
        E = new pb("achievement.enchantments", "enchantments", -4, 4, ajn.bn, w).c();
        F = new pb("achievement.overkill", "overkill", -4, 1, ade.u, E).b().c();
        G = new pb("achievement.bookcase", "bookcase", -3, 6, ajn.X, E).c();
        H = new pb("achievement.breedCow", "breedCow", 7, -5, ade.O, t).c();
        I = new pb("achievement.spawnWither", "spawnWither", 7, 12, new add(ade.bL, 1, 1), D).c();
        J = new pb("achievement.killWither", "killWither", 7, 10, ade.bN, I).c();
        K = new pb("achievement.fullBeacon", "fullBeacon", 7, 8, ajn.bJ, J).b().c();
        L = new pb("achievement.exploreAllBiomes", "exploreAllBiomes", 4, 8, ade.af, C).a(pr.class).b().c();
    }
}

