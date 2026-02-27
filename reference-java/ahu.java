/*
 * Decompiled with CFR 0.152.
 */
import com.google.common.collect.Sets;
import java.util.ArrayList;
import java.util.List;
import java.util.Random;
import java.util.Set;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public abstract class ahu {
    private static final Logger aC = LogManager.getLogger();
    protected static final ahv a = new ahv(0.1f, 0.2f);
    protected static final ahv b = new ahv(-0.5f, 0.0f);
    protected static final ahv c = new ahv(-1.0f, 0.1f);
    protected static final ahv d = new ahv(-1.8f, 0.1f);
    protected static final ahv e = new ahv(0.125f, 0.05f);
    protected static final ahv f = new ahv(0.2f, 0.2f);
    protected static final ahv g = new ahv(0.45f, 0.3f);
    protected static final ahv h = new ahv(1.5f, 0.025f);
    protected static final ahv i = new ahv(1.0f, 0.5f);
    protected static final ahv j = new ahv(0.0f, 0.025f);
    protected static final ahv k = new ahv(0.1f, 0.8f);
    protected static final ahv l = new ahv(0.2f, 0.3f);
    protected static final ahv m = new ahv(-0.2f, 0.1f);
    private static final ahu[] aD = new ahu[256];
    public static final Set n = Sets.newHashSet();
    public static final ahu o = new aio(0).b(112).a("Ocean").a(c);
    public static final ahu p = new aip(1).b(9286496).a("Plains");
    public static final ahu q = new aic(2).b(16421912).a("Desert").b().a(2.0f, 0.0f).a(e);
    public static final ahu r = new aid(3, false).b(0x606060).a("Extreme Hills").a(i).a(0.2f, 0.3f);
    public static final ahu s = new aif(4, 0).b(353825).a("Forest");
    public static final ahu t = new aiw(5, 0).b(747097).a("Taiga").a(5159473).a(0.25f, 0.8f).a(f);
    public static final ahu u = new aiv(6).b(522674).a("Swampland").a(9154376).a(m).a(0.8f, 0.9f);
    public static final ahu v = new air(7).b(255).a("River").a(b);
    public static final ahu w = new aii(8).b(0xFF0000).a("Hell").b().a(2.0f, 0.0f);
    public static final ahu x = new aix(9).b(0x8080FF).a("Sky").b();
    public static final ahu y = new aio(10).b(0x9090A0).a("FrozenOcean").c().a(c).a(0.0f, 0.5f);
    public static final ahu z = new air(11).b(0xA0A0FF).a("FrozenRiver").c().a(b).a(0.0f, 0.5f);
    public static final ahu A = new aij(12, false).b(0xFFFFFF).a("Ice Plains").c().a(0.0f, 0.5f).a(e);
    public static final ahu B = new aij(13, false).b(0xA0A0A0).a("Ice Mountains").c().a(g).a(0.0f, 0.5f);
    public static final ahu C = new aim(14).b(0xFF00FF).a("MushroomIsland").a(0.9f, 1.0f).a(l);
    public static final ahu D = new aim(15).b(0xA000FF).a("MushroomIslandShore").a(0.9f, 1.0f).a(j);
    public static final ahu E = new aht(16).b(16440917).a("Beach").a(0.8f, 0.4f).a(j);
    public static final ahu F = new aic(17).b(13786898).a("DesertHills").b().a(2.0f, 0.0f).a(g);
    public static final ahu G = new aif(18, 0).b(2250012).a("ForestHills").a(g);
    public static final ahu H = new aiw(19, 0).b(1456435).a("TaigaHills").a(5159473).a(0.25f, 0.8f).a(g);
    public static final ahu I = new aid(20, true).b(7501978).a("Extreme Hills Edge").a(i.a()).a(0.2f, 0.3f);
    public static final ahu J = new aik(21, false).b(5470985).a("Jungle").a(5470985).a(0.95f, 0.9f);
    public static final ahu K = new aik(22, false).b(2900485).a("JungleHills").a(5470985).a(0.95f, 0.9f).a(g);
    public static final ahu L = new aik(23, true).b(6458135).a("JungleEdge").a(5470985).a(0.95f, 0.8f);
    public static final ahu M = new aio(24).b(48).a("Deep Ocean").a(d);
    public static final ahu N = new aiu(25).b(10658436).a("Stone Beach").a(0.2f, 0.3f).a(k);
    public static final ahu O = new aht(26).b(16445632).a("Cold Beach").a(0.05f, 0.3f).a(j).c();
    public static final ahu P = new aif(27, 2).a("Birch Forest").b(3175492);
    public static final ahu Q = new aif(28, 2).a("Birch Forest Hills").b(2055986).a(g);
    public static final ahu R = new aif(29, 3).b(4215066).a("Roofed Forest");
    public static final ahu S = new aiw(30, 0).b(3233098).a("Cold Taiga").a(5159473).c().a(-0.5f, 0.4f).a(f).c(0xFFFFFF);
    public static final ahu T = new aiw(31, 0).b(2375478).a("Cold Taiga Hills").a(5159473).c().a(-0.5f, 0.4f).a(g).c(0xFFFFFF);
    public static final ahu U = new aiw(32, 1).b(5858897).a("Mega Taiga").a(5159473).a(0.3f, 0.8f).a(f);
    public static final ahu V = new aiw(33, 1).b(4542270).a("Mega Taiga Hills").a(5159473).a(0.3f, 0.8f).a(g);
    public static final ahu W = new aid(34, true).b(0x507050).a("Extreme Hills+").a(i).a(0.2f, 0.3f);
    public static final ahu X = new ais(35).b(12431967).a("Savanna").a(1.2f, 0.0f).b().a(e);
    public static final ahu Y = new ais(36).b(10984804).a("Savanna Plateau").a(1.0f, 0.0f).b().a(h);
    public static final ahu Z = new ail(37, false, false).b(14238997).a("Mesa");
    public static final ahu aa = new ail(38, false, true).b(11573093).a("Mesa Plateau F").a(h);
    public static final ahu ab = new ail(39, false, false).b(13274213).a("Mesa Plateau").a(h);
    protected static final awl ac;
    protected static final awl ad;
    protected static final arm ae;
    public String af;
    public int ag;
    public int ah;
    public aji ai = ajn.c;
    public int aj = 0;
    public aji ak = ajn.d;
    public int al = 5169201;
    public float am;
    public float an;
    public float ao;
    public float ap;
    public int aq;
    public aia ar;
    protected List as;
    protected List at;
    protected List au;
    protected List av;
    protected boolean aw;
    protected boolean ax;
    public final int ay;
    protected asq az;
    protected ard aA;
    protected aso aB;

    protected ahu(int n2) {
        this.am = ahu.a.a;
        this.an = ahu.a.b;
        this.ao = 0.5f;
        this.ap = 0.5f;
        this.aq = 0xFFFFFF;
        this.as = new ArrayList();
        this.at = new ArrayList();
        this.au = new ArrayList();
        this.av = new ArrayList();
        this.ax = true;
        this.az = new asq(false);
        this.aA = new ard(false);
        this.aB = new aso();
        this.ay = n2;
        ahu.aD[n2] = this;
        this.ar = this.a();
        this.at.add(new ahx(wp.class, 12, 4, 4));
        this.at.add(new ahx(wo.class, 10, 4, 4));
        this.at.add(new ahx(wg.class, 10, 4, 4));
        this.at.add(new ahx(wh.class, 8, 4, 4));
        this.as.add(new ahx(yn.class, 100, 4, 4));
        this.as.add(new ahx(yq.class, 100, 4, 4));
        this.as.add(new ahx(yl.class, 100, 4, 4));
        this.as.add(new ahx(xz.class, 100, 4, 4));
        this.as.add(new ahx(ym.class, 100, 4, 4));
        this.as.add(new ahx(ya.class, 10, 1, 4));
        this.as.add(new ahx(yp.class, 5, 1, 1));
        this.au.add(new ahx(ws.class, 10, 4, 4));
        this.av.add(new ahx(we.class, 10, 8, 8));
    }

    protected aia a() {
        return new aia();
    }

    protected ahu a(float f2, float f3) {
        if (f2 > 0.1f && f2 < 0.2f) {
            throw new IllegalArgumentException("Please avoid temperatures in the range 0.1 - 0.2 because of snow");
        }
        this.ao = f2;
        this.ap = f3;
        return this;
    }

    protected final ahu a(ahv ahv2) {
        this.am = ahv2.a;
        this.an = ahv2.b;
        return this;
    }

    protected ahu b() {
        this.ax = false;
        return this;
    }

    public arc a(Random random) {
        if (random.nextInt(10) == 0) {
            return this.aA;
        }
        return this.az;
    }

    public arn b(Random random) {
        return new asp(ajn.H, 1);
    }

    public String a(Random random, int n2, int n3, int n4) {
        if (random.nextInt(3) > 0) {
            return alc.b[0];
        }
        return alc.a[0];
    }

    protected ahu c() {
        this.aw = true;
        return this;
    }

    protected ahu a(String string) {
        this.af = string;
        return this;
    }

    protected ahu a(int n2) {
        this.al = n2;
        return this;
    }

    protected ahu b(int n2) {
        this.a(n2, false);
        return this;
    }

    protected ahu c(int n2) {
        this.ah = n2;
        return this;
    }

    protected ahu a(int n2, boolean bl2) {
        this.ag = n2;
        this.ah = bl2 ? (n2 & 0xFEFEFE) >> 1 : n2;
        return this;
    }

    public List a(sx sx2) {
        if (sx2 == sx.a) {
            return this.as;
        }
        if (sx2 == sx.b) {
            return this.at;
        }
        if (sx2 == sx.d) {
            return this.au;
        }
        if (sx2 == sx.c) {
            return this.av;
        }
        return null;
    }

    public boolean d() {
        return this.j();
    }

    public boolean e() {
        if (this.j()) {
            return false;
        }
        return this.ax;
    }

    public boolean f() {
        return this.ap > 0.85f;
    }

    public float g() {
        return 0.1f;
    }

    public final int h() {
        return (int)(this.ap * 65536.0f);
    }

    public final float a(int n2, int n3, int n4) {
        if (n3 > 64) {
            float f2 = (float)ac.a((double)n2 * 1.0 / 8.0, (double)n4 * 1.0 / 8.0) * 4.0f;
            return this.ao - (f2 + (float)n3 - 64.0f) * 0.05f / 30.0f;
        }
        return this.ao;
    }

    public void a(ahb ahb2, Random random, int n2, int n3) {
        this.ar.a(ahb2, random, this, n2, n3);
    }

    public boolean j() {
        return this.aw;
    }

    public void a(ahb ahb2, Random random, aji[] ajiArray, byte[] byArray, int n2, int n3, double d2) {
        this.b(ahb2, random, ajiArray, byArray, n2, n3, d2);
    }

    public final void b(ahb ahb2, Random random, aji[] ajiArray, byte[] byArray, int n2, int n3, double d2) {
        int n4 = 63;
        aji aji2 = this.ai;
        byte by2 = (byte)(this.aj & 0xFF);
        aji aji3 = this.ak;
        int n5 = -1;
        int n6 = (int)(d2 / 3.0 + 3.0 + random.nextDouble() * 0.25);
        int n7 = n2 & 0xF;
        int n8 = n3 & 0xF;
        int n9 = ajiArray.length / 256;
        for (int i2 = 255; i2 >= 0; --i2) {
            int n10 = (n8 * 16 + n7) * n9 + i2;
            if (i2 <= 0 + random.nextInt(5)) {
                ajiArray[n10] = ajn.h;
                continue;
            }
            aji aji4 = ajiArray[n10];
            if (aji4 == null || aji4.o() == awt.a) {
                n5 = -1;
                continue;
            }
            if (aji4 != ajn.b) continue;
            if (n5 == -1) {
                if (n6 <= 0) {
                    aji2 = null;
                    by2 = 0;
                    aji3 = ajn.b;
                } else if (i2 >= 59 && i2 <= 64) {
                    aji2 = this.ai;
                    by2 = (byte)(this.aj & 0xFF);
                    aji3 = this.ak;
                }
                if (i2 < 63 && (aji2 == null || aji2.o() == awt.a)) {
                    if (this.a(n2, i2, n3) < 0.15f) {
                        aji2 = ajn.aD;
                        by2 = 0;
                    } else {
                        aji2 = ajn.j;
                        by2 = 0;
                    }
                }
                n5 = n6;
                if (i2 >= 62) {
                    ajiArray[n10] = aji2;
                    byArray[n10] = by2;
                    continue;
                }
                if (i2 < 56 - n6) {
                    aji2 = null;
                    aji3 = ajn.b;
                    ajiArray[n10] = ajn.n;
                    continue;
                }
                ajiArray[n10] = aji3;
                continue;
            }
            if (n5 <= 0) continue;
            ajiArray[n10] = aji3;
            if (--n5 != 0 || aji3 != ajn.m) continue;
            n5 = random.nextInt(4) + Math.max(0, i2 - 63);
            aji3 = ajn.A;
        }
    }

    protected ahu k() {
        return new ain(this.ay + 128, this);
    }

    public Class l() {
        return this.getClass();
    }

    public boolean a(ahu ahu2) {
        if (ahu2 == this) {
            return true;
        }
        if (ahu2 == null) {
            return false;
        }
        return this.l() == ahu2.l();
    }

    public ahw m() {
        if ((double)this.ao < 0.2) {
            return ahw.b;
        }
        if ((double)this.ao < 1.0) {
            return ahw.c;
        }
        return ahw.d;
    }

    public static ahu[] n() {
        return aD;
    }

    public static ahu d(int n2) {
        if (n2 < 0 || n2 > aD.length) {
            aC.warn("Biome ID is out of bounds: " + n2 + ", defaulting to 0 (Ocean)");
            return o;
        }
        return aD[n2];
    }

    static {
        p.k();
        q.k();
        s.k();
        t.k();
        u.k();
        A.k();
        J.k();
        L.k();
        S.k();
        X.k();
        Y.k();
        Z.k();
        aa.k();
        ab.k();
        P.k();
        Q.k();
        R.k();
        U.k();
        r.k();
        W.k();
        ahu.aD[ahu.V.ay + 128] = aD[ahu.U.ay + 128];
        for (ahu ahu2 : aD) {
            if (ahu2 == null || ahu2.ay >= 128) continue;
            n.add(ahu2);
        }
        n.remove(w);
        n.remove(x);
        n.remove(y);
        n.remove(I);
        ac = new awl(new Random(1234L), 1);
        ad = new awl(new Random(2345L), 1);
        ae = new arm();
    }
}

