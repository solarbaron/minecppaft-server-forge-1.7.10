/*
 * Decompiled with CFR 0.152.
 */
import com.google.common.collect.Maps;
import java.util.Map;
import java.util.UUID;

public class rv {
    public static final rv[] a = new rv[32];
    public static final rv b = null;
    public static final rv c = new rv(1, false, 8171462).b("potion.moveSpeed").b(0, 0).a(yj.d, "91AEAA56-376B-4498-935B-2F7F68070635", 0.2f, 2);
    public static final rv d = new rv(2, true, 5926017).b("potion.moveSlowdown").b(1, 0).a(yj.d, "7107DE5E-7CE8-4030-940E-514C1F160890", -0.15f, 2);
    public static final rv e = new rv(3, false, 14270531).b("potion.digSpeed").b(2, 0).a(1.5);
    public static final rv f = new rv(4, true, 4866583).b("potion.digSlowDown").b(3, 0);
    public static final rv g = new rs(5, false, 9643043).b("potion.damageBoost").b(4, 0).a(yj.e, "648D7064-6A60-4F59-8ABE-C2C23A6DD7A9", 3.0, 2);
    public static final rv h = new ru(6, false, 16262179).b("potion.heal");
    public static final rv i = new ru(7, true, 4393481).b("potion.harm");
    public static final rv j = new rv(8, false, 7889559).b("potion.jump").b(2, 1);
    public static final rv k = new rv(9, true, 5578058).b("potion.confusion").b(3, 1).a(0.25);
    public static final rv l = new rv(10, false, 13458603).b("potion.regeneration").b(7, 0).a(0.25);
    public static final rv m = new rv(11, false, 10044730).b("potion.resistance").b(6, 1);
    public static final rv n = new rv(12, false, 14981690).b("potion.fireResistance").b(7, 1);
    public static final rv o = new rv(13, false, 3035801).b("potion.waterBreathing").b(0, 2);
    public static final rv p = new rv(14, false, 8356754).b("potion.invisibility").b(0, 1);
    public static final rv q = new rv(15, true, 2039587).b("potion.blindness").b(5, 1).a(0.25);
    public static final rv r = new rv(16, false, 0x1F1FA1).b("potion.nightVision").b(4, 1);
    public static final rv s = new rv(17, true, 5797459).b("potion.hunger").b(1, 1);
    public static final rv t = new rs(18, true, 0x484D48).b("potion.weakness").b(5, 0).a(yj.e, "22653B89-116E-49DC-9B6B-9971489B5BE5", 2.0, 0);
    public static final rv u = new rv(19, true, 5149489).b("potion.poison").b(6, 0).a(0.25);
    public static final rv v = new rv(20, true, 3484199).b("potion.wither").b(1, 2).a(0.25);
    public static final rv w = new rt(21, false, 16284963).b("potion.healthBoost").b(2, 2).a(yj.a, "5D6F0BA2-1186-46AC-B896-C61C5CEE99CC", 4.0, 0);
    public static final rv x = new rr(22, false, 0x2552A5).b("potion.absorption").b(2, 2);
    public static final rv y = new ru(23, false, 16262179).b("potion.saturation");
    public static final rv z = null;
    public static final rv A = null;
    public static final rv B = null;
    public static final rv C = null;
    public static final rv D = null;
    public static final rv E = null;
    public static final rv F = null;
    public static final rv G = null;
    public final int H;
    private final Map I = Maps.newHashMap();
    private final boolean J;
    private final int K;
    private String L = "";
    private int M = -1;
    private double N;
    private boolean O;

    protected rv(int n2, boolean bl2, int n3) {
        this.H = n2;
        rv.a[n2] = this;
        this.J = bl2;
        this.N = bl2 ? 0.5 : 1.0;
        this.K = n3;
    }

    protected rv b(int n2, int n3) {
        this.M = n2 + n3 * 8;
        return this;
    }

    public int c() {
        return this.H;
    }

    public void a(sv sv2, int n2) {
        if (this.H == rv.l.H) {
            if (sv2.aS() < sv2.aY()) {
                sv2.f(1.0f);
            }
        } else if (this.H == rv.u.H) {
            if (sv2.aS() > 1.0f) {
                sv2.a(ro.k, 1.0f);
            }
        } else if (this.H == rv.v.H) {
            sv2.a(ro.l, 1.0f);
        } else if (this.H == rv.s.H && sv2 instanceof yz) {
            ((yz)sv2).a(0.025f * (float)(n2 + 1));
        } else if (this.H == rv.y.H && sv2 instanceof yz) {
            if (!sv2.o.E) {
                ((yz)sv2).bQ().a(n2 + 1, 1.0f);
            }
        } else if (this.H == rv.h.H && !sv2.aR() || this.H == rv.i.H && sv2.aR()) {
            sv2.f((float)Math.max(4 << n2, 0));
        } else if (this.H == rv.i.H && !sv2.aR() || this.H == rv.h.H && sv2.aR()) {
            sv2.a(ro.k, (float)(6 << n2));
        }
    }

    public void a(sv sv2, sv sv3, int n2, double d2) {
        if (this.H == rv.h.H && !sv3.aR() || this.H == rv.i.H && sv3.aR()) {
            int n3 = (int)(d2 * (double)(4 << n2) + 0.5);
            sv3.f((float)n3);
        } else if (this.H == rv.i.H && !sv3.aR() || this.H == rv.h.H && sv3.aR()) {
            int n4 = (int)(d2 * (double)(6 << n2) + 0.5);
            if (sv2 == null) {
                sv3.a(ro.k, (float)n4);
            } else {
                sv3.a(ro.b(sv3, sv2), (float)n4);
            }
        }
    }

    public boolean b() {
        return false;
    }

    public boolean a(int n2, int n3) {
        if (this.H == rv.l.H) {
            int n4 = 50 >> n3;
            if (n4 > 0) {
                return n2 % n4 == 0;
            }
            return true;
        }
        if (this.H == rv.u.H) {
            int n5 = 25 >> n3;
            if (n5 > 0) {
                return n2 % n5 == 0;
            }
            return true;
        }
        if (this.H == rv.v.H) {
            int n6 = 40 >> n3;
            if (n6 > 0) {
                return n2 % n6 == 0;
            }
            return true;
        }
        return this.H == rv.s.H;
    }

    public rv b(String string) {
        this.L = string;
        return this;
    }

    public String a() {
        return this.L;
    }

    protected rv a(double d2) {
        this.N = d2;
        return this;
    }

    public double g() {
        return this.N;
    }

    public boolean i() {
        return this.O;
    }

    public int j() {
        return this.K;
    }

    public rv a(th th2, String string, double d2, int n2) {
        tj tj2 = new tj(UUID.fromString(string), this.a(), d2, n2);
        this.I.put(th2, tj2);
        return this;
    }

    public void a(sv sv2, tl tl2, int n2) {
        for (Map.Entry entry : this.I.entrySet()) {
            ti ti2 = tl2.a((th)entry.getKey());
            if (ti2 == null) continue;
            ti2.b((tj)entry.getValue());
        }
    }

    public void b(sv sv2, tl tl2, int n2) {
        for (Map.Entry entry : this.I.entrySet()) {
            ti ti2 = tl2.a((th)entry.getKey());
            if (ti2 == null) continue;
            tj tj2 = (tj)entry.getValue();
            ti2.b(tj2);
            ti2.a(new tj(tj2.a(), this.a() + " " + n2, this.a(n2, tj2), tj2.c()));
        }
    }

    public double a(int n2, tj tj2) {
        return tj2.d() * (double)(n2 + 1);
    }
}

