/*
 * Decompiled with CFR 0.152.
 */
import com.google.common.collect.Lists;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Random;
import java.util.Set;
import java.util.TreeSet;
import net.minecraft.server.MinecraftServer;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class mt
extends ahb {
    private static final Logger a = LogManager.getLogger();
    private final MinecraftServer J;
    private final mn K;
    private final mq L;
    private Set M;
    private TreeSet N;
    public ms b;
    public boolean c;
    private boolean O;
    private int P;
    private final ahp Q;
    private final aho R = new aho();
    private mv[] S = new mv[]{new mv(null), new mv(null)};
    private int T;
    private static final qx[] U = new qx[]{new qx(ade.y, 0, 1, 3, 10), new qx(adb.a(ajn.f), 0, 1, 3, 10), new qx(adb.a(ajn.r), 0, 1, 3, 10), new qx(ade.t, 0, 1, 1, 3), new qx(ade.p, 0, 1, 1, 5), new qx(ade.s, 0, 1, 1, 3), new qx(ade.o, 0, 1, 1, 5), new qx(ade.e, 0, 2, 3, 5), new qx(ade.P, 0, 2, 3, 3), new qx(adb.a(ajn.s), 0, 1, 3, 10)};
    private List V = new ArrayList();
    private pz W;

    public mt(MinecraftServer minecraftServer, azc azc2, String string, int n2, ahj ahj2, qi qi2) {
        super(azc2, string, ahj2, aqo.a(n2), qi2);
        this.J = minecraftServer;
        this.K = new mn(this);
        this.L = new mq(this);
        if (this.W == null) {
            this.W = new pz();
        }
        if (this.M == null) {
            this.M = new HashSet();
        }
        if (this.N == null) {
            this.N = new TreeSet();
        }
        this.Q = new ahp(this);
        this.D = new lp(minecraftServer);
        bad bad2 = (bad)this.z.a(bad.class, "scoreboard");
        if (bad2 == null) {
            bad2 = new bad();
            this.z.a("scoreboard", bad2);
        }
        bad2.a(this.D);
        ((lp)this.D).a(bad2);
    }

    @Override
    public void b() {
        super.b();
        if (this.N().t() && this.r != rd.d) {
            this.r = rd.d;
        }
        this.t.e.b();
        if (this.e()) {
            if (this.O().b("doDaylightCycle")) {
                long l2 = this.x.g() + 24000L;
                this.x.c(l2 - l2 % 24000L);
            }
            this.d();
        }
        this.C.a("mobSpawner");
        if (this.O().b("doMobSpawning")) {
            this.R.a(this, this.G, this.H, this.x.f() % 400L == 0L);
        }
        this.C.c("chunkSource");
        this.v.d();
        int n2 = this.a(1.0f);
        if (n2 != this.j) {
            this.j = n2;
        }
        this.x.b(this.x.f() + 1L);
        if (this.O().b("doDaylightCycle")) {
            this.x.c(this.x.g() + 1L);
        }
        this.C.c("tickPending");
        this.a(false);
        this.C.c("tickBlocks");
        this.g();
        this.C.c("chunkMap");
        this.L.b();
        this.C.c("village");
        this.A.a();
        this.B.a();
        this.C.c("portalForcer");
        this.Q.a(this.I());
        this.C.b();
        this.Z();
    }

    public ahx a(sx sx2, int n2, int n3, int n4) {
        List list = this.L().a(sx2, n2, n3, n4);
        if (list == null || list.isEmpty()) {
            return null;
        }
        return (ahx)qv.a(this.s, list);
    }

    @Override
    public void c() {
        this.O = !this.h.isEmpty();
        for (yz yz2 : this.h) {
            if (yz2.bm()) continue;
            this.O = false;
            break;
        }
    }

    protected void d() {
        this.O = false;
        for (yz yz2 : this.h) {
            if (!yz2.bm()) continue;
            yz2.a(false, false, true);
        }
        this.Y();
    }

    private void Y() {
        this.x.g(0);
        this.x.b(false);
        this.x.f(0);
        this.x.a(false);
    }

    public boolean e() {
        if (this.O && !this.E) {
            for (yz yz2 : this.h) {
                if (yz2.bL()) continue;
                return false;
            }
            return true;
        }
        return false;
    }

    @Override
    protected void g() {
        super.g();
        int n2 = 0;
        int n3 = 0;
        for (agu agu2 : this.F) {
            int n4;
            int n5;
            int n6;
            int n7;
            int n8 = agu2.a * 16;
            int n9 = agu2.b * 16;
            this.C.a("getChunk");
            apx apx2 = this.e(agu2.a, agu2.b);
            this.a(n8, n9, apx2);
            this.C.c("tickChunk");
            apx2.b(false);
            this.C.c("thunder");
            if (this.s.nextInt(100000) == 0 && this.Q() && this.P()) {
                this.k = this.k * 3 + 1013904223;
                n7 = this.k >> 2;
                n6 = n8 + (n7 & 0xF);
                n5 = n9 + (n7 >> 8 & 0xF);
                n4 = this.h(n6, n5);
                if (this.y(n6, n4, n5)) {
                    this.c(new xh(this, n6, n4, n5));
                }
            }
            this.C.c("iceandsnow");
            if (this.s.nextInt(16) == 0) {
                ahu ahu2;
                this.k = this.k * 3 + 1013904223;
                n7 = this.k >> 2;
                n6 = n7 & 0xF;
                n5 = n7 >> 8 & 0xF;
                n4 = this.h(n6 + n8, n5 + n9);
                if (this.s(n6 + n8, n4 - 1, n5 + n9)) {
                    this.b(n6 + n8, n4 - 1, n5 + n9, ajn.aD);
                }
                if (this.Q() && this.e(n6 + n8, n4, n5 + n9, true)) {
                    this.b(n6 + n8, n4, n5 + n9, ajn.aC);
                }
                if (this.Q() && (ahu2 = this.a(n6 + n8, n5 + n9)).e()) {
                    this.a(n6 + n8, n4 - 1, n5 + n9).l(this, n6 + n8, n4 - 1, n5 + n9);
                }
            }
            this.C.c("tickBlocks");
            for (apz apz2 : apx2.i()) {
                if (apz2 == null || !apz2.b()) continue;
                for (int i2 = 0; i2 < 3; ++i2) {
                    this.k = this.k * 3 + 1013904223;
                    int n10 = this.k >> 2;
                    int n11 = n10 & 0xF;
                    int n12 = n10 >> 8 & 0xF;
                    int n13 = n10 >> 16 & 0xF;
                    ++n3;
                    aji aji2 = apz2.a(n11, n13, n12);
                    if (!aji2.t()) continue;
                    ++n2;
                    aji2.a((ahb)this, n11 + n8, n13 + apz2.d(), n12 + n9, this.s);
                }
            }
            this.C.b();
        }
    }

    @Override
    public boolean a(int n2, int n3, int n4, aji aji2) {
        ahs ahs2 = new ahs(n2, n3, n4, aji2);
        return this.V.contains(ahs2);
    }

    @Override
    public void a(int n2, int n3, int n4, aji aji2, int n5) {
        this.a(n2, n3, n4, aji2, n5, 0);
    }

    @Override
    public void a(int n2, int n3, int n4, aji aji2, int n5, int n6) {
        ahs ahs2 = new ahs(n2, n3, n4, aji2);
        int n7 = 0;
        if (this.d && aji2.o() != awt.a) {
            if (aji2.L()) {
                aji aji3;
                n7 = 8;
                if (this.b(ahs2.a - n7, ahs2.b - n7, ahs2.c - n7, ahs2.a + n7, ahs2.b + n7, ahs2.c + n7) && (aji3 = this.a(ahs2.a, ahs2.b, ahs2.c)).o() != awt.a && aji3 == ahs2.a()) {
                    aji3.a((ahb)this, ahs2.a, ahs2.b, ahs2.c, this.s);
                }
                return;
            }
            n5 = 1;
        }
        if (this.b(n2 - n7, n3 - n7, n4 - n7, n2 + n7, n3 + n7, n4 + n7)) {
            if (aji2.o() != awt.a) {
                ahs2.a((long)n5 + this.x.f());
                ahs2.a(n6);
            }
            if (!this.M.contains(ahs2)) {
                this.M.add(ahs2);
                this.N.add(ahs2);
            }
        }
    }

    @Override
    public void b(int n2, int n3, int n4, aji aji2, int n5, int n6) {
        ahs ahs2 = new ahs(n2, n3, n4, aji2);
        ahs2.a(n6);
        if (aji2.o() != awt.a) {
            ahs2.a((long)n5 + this.x.f());
        }
        if (!this.M.contains(ahs2)) {
            this.M.add(ahs2);
            this.N.add(ahs2);
        }
    }

    @Override
    public void h() {
        if (this.h.isEmpty()) {
            if (this.P++ >= 1200) {
                return;
            }
        } else {
            this.i();
        }
        super.h();
    }

    public void i() {
        this.P = 0;
    }

    @Override
    public boolean a(boolean bl2) {
        ahs ahs2;
        int n2 = this.N.size();
        if (n2 != this.M.size()) {
            throw new IllegalStateException("TickNextTick list out of synch");
        }
        if (n2 > 1000) {
            n2 = 1000;
        }
        this.C.a("cleaning");
        for (int i2 = 0; i2 < n2; ++i2) {
            ahs2 = (ahs)this.N.first();
            if (!bl2 && ahs2.d > this.x.f()) break;
            this.N.remove(ahs2);
            this.M.remove(ahs2);
            this.V.add(ahs2);
        }
        this.C.b();
        this.C.a("ticking");
        Iterator iterator = this.V.iterator();
        while (iterator.hasNext()) {
            ahs2 = (ahs)iterator.next();
            iterator.remove();
            int n3 = 0;
            if (this.b(ahs2.a - n3, ahs2.b - n3, ahs2.c - n3, ahs2.a + n3, ahs2.b + n3, ahs2.c + n3)) {
                aji aji2 = this.a(ahs2.a, ahs2.b, ahs2.c);
                if (aji2.o() == awt.a || !aji.a(aji2, ahs2.a())) continue;
                try {
                    aji2.a((ahb)this, ahs2.a, ahs2.b, ahs2.c, this.s);
                    continue;
                }
                catch (Throwable throwable) {
                    int n4;
                    b b2 = b.a(throwable, "Exception while ticking a block");
                    k k2 = b2.a("Block being ticked");
                    try {
                        n4 = this.e(ahs2.a, ahs2.b, ahs2.c);
                    }
                    catch (Throwable throwable2) {
                        n4 = -1;
                    }
                    k.a(k2, ahs2.a, ahs2.b, ahs2.c, aji2, n4);
                    throw new s(b2);
                }
            }
            this.a(ahs2.a, ahs2.b, ahs2.c, ahs2.a(), 0);
        }
        this.C.b();
        this.V.clear();
        return !this.N.isEmpty();
    }

    @Override
    public List a(apx apx2, boolean bl2) {
        ArrayList<ahs> arrayList = null;
        agu agu2 = apx2.l();
        int n2 = (agu2.a << 4) - 2;
        int n3 = n2 + 16 + 2;
        int n4 = (agu2.b << 4) - 2;
        int n5 = n4 + 16 + 2;
        for (int i2 = 0; i2 < 2; ++i2) {
            Iterator iterator;
            if (i2 == 0) {
                iterator = this.N.iterator();
            } else {
                iterator = this.V.iterator();
                if (!this.V.isEmpty()) {
                    a.debug("toBeTicked = " + this.V.size());
                }
            }
            while (iterator.hasNext()) {
                ahs ahs2 = (ahs)iterator.next();
                if (ahs2.a < n2 || ahs2.a >= n3 || ahs2.c < n4 || ahs2.c >= n5) continue;
                if (bl2) {
                    this.M.remove(ahs2);
                    iterator.remove();
                }
                if (arrayList == null) {
                    arrayList = new ArrayList<ahs>();
                }
                arrayList.add(ahs2);
            }
        }
        return arrayList;
    }

    @Override
    public void a(sa sa2, boolean bl2) {
        if (!this.J.Z() && (sa2 instanceof wf || sa2 instanceof wu)) {
            sa2.B();
        }
        if (!this.J.aa() && sa2 instanceof yu) {
            sa2.B();
        }
        super.a(sa2, bl2);
    }

    @Override
    protected apu j() {
        aqc aqc2 = this.w.a(this.t);
        this.b = new ms(this, aqc2, this.t.c());
        return this.b;
    }

    public List a(int n2, int n3, int n4, int n5, int n6, int n7) {
        ArrayList<aor> arrayList = new ArrayList<aor>();
        for (int i2 = 0; i2 < this.g.size(); ++i2) {
            aor aor2 = (aor)this.g.get(i2);
            if (aor2.c < n2 || aor2.d < n3 || aor2.e < n4 || aor2.c >= n5 || aor2.d >= n6 || aor2.e >= n7) continue;
            arrayList.add(aor2);
        }
        return arrayList;
    }

    @Override
    public boolean a(yz yz2, int n2, int n3, int n4) {
        return !this.J.a(this, n2, n3, n4, yz2);
    }

    @Override
    protected void a(ahj ahj2) {
        if (this.W == null) {
            this.W = new pz();
        }
        if (this.M == null) {
            this.M = new HashSet();
        }
        if (this.N == null) {
            this.N = new TreeSet();
        }
        this.b(ahj2);
        super.a(ahj2);
    }

    protected void b(ahj ahj2) {
        if (!this.t.e()) {
            this.x.a(0, this.t.i(), 0);
            return;
        }
        this.y = true;
        aib aib2 = this.t.e;
        List list = aib2.a();
        Random random = new Random(this.H());
        agt agt2 = aib2.a(0, 0, 256, list, random);
        int n2 = 0;
        int n3 = this.t.i();
        int n4 = 0;
        if (agt2 != null) {
            n2 = agt2.a;
            n4 = agt2.c;
        } else {
            a.warn("Unable to find spawn biome");
        }
        int n5 = 0;
        while (!this.t.a(n2, n4)) {
            n2 += random.nextInt(64) - random.nextInt(64);
            n4 += random.nextInt(64) - random.nextInt(64);
            if (++n5 != 1000) continue;
        }
        this.x.a(n2, n3, n4);
        this.y = false;
        if (ahj2.c()) {
            this.k();
        }
    }

    protected void k() {
        int n2;
        int n3;
        int n4;
        arg arg2 = new arg(U, 10);
        for (int i2 = 0; i2 < 10 && !arg2.a((ahb)this, this.s, n4 = this.x.c() + this.s.nextInt(6) - this.s.nextInt(6), n3 = this.i(n4, n2 = this.x.e() + this.s.nextInt(6) - this.s.nextInt(6)) + 1, n2); ++i2) {
        }
    }

    public r l() {
        return this.t.h();
    }

    public void a(boolean bl2, qk qk2) {
        if (!this.v.e()) {
            return;
        }
        if (qk2 != null) {
            qk2.a("Saving level");
        }
        this.a();
        if (qk2 != null) {
            qk2.c("Saving chunks");
        }
        this.v.a(bl2, qk2);
        ArrayList<apx> arrayList = Lists.newArrayList(this.b.a());
        for (apx apx2 : arrayList) {
            if (apx2 == null || this.L.a(apx2.g, apx2.h)) continue;
            this.b.b(apx2.g, apx2.h);
        }
    }

    public void m() {
        if (!this.v.e()) {
            return;
        }
        this.v.c();
    }

    protected void a() {
        this.G();
        this.w.a(this.x, this.J.ah().t());
        this.z.a();
    }

    @Override
    protected void a(sa sa2) {
        super.a(sa2);
        this.W.a(sa2.y(), sa2);
        sa[] saArray = sa2.at();
        if (saArray != null) {
            for (int i2 = 0; i2 < saArray.length; ++i2) {
                this.W.a(saArray[i2].y(), saArray[i2]);
            }
        }
    }

    @Override
    protected void b(sa sa2) {
        super.b(sa2);
        this.W.d(sa2.y());
        sa[] saArray = sa2.at();
        if (saArray != null) {
            for (int i2 = 0; i2 < saArray.length; ++i2) {
                this.W.d(saArray[i2].y());
            }
        }
    }

    @Override
    public sa a(int n2) {
        return (sa)this.W.a(n2);
    }

    @Override
    public boolean c(sa sa2) {
        if (super.c(sa2)) {
            this.J.ah().a(sa2.s, sa2.t, sa2.u, 512.0, this.t.i, new fy(sa2));
            return true;
        }
        return false;
    }

    @Override
    public void a(sa sa2, byte by2) {
        this.r().b(sa2, new gt(sa2, by2));
    }

    @Override
    public agw a(sa sa2, double d2, double d3, double d4, float f2, boolean bl2, boolean bl3) {
        agw agw2 = new agw(this, sa2, d2, d3, d4, f2);
        agw2.a = bl2;
        agw2.b = bl3;
        agw2.a();
        agw2.a(false);
        if (!bl3) {
            agw2.h.clear();
        }
        for (yz yz2 : this.h) {
            if (!(yz2.e(d2, d3, d4) < 4096.0)) continue;
            ((mw)yz2).a.a(new gu(d2, d3, d4, f2, agw2.h, (azw)agw2.b().get(yz2)));
        }
        return agw2;
    }

    @Override
    public void c(int n2, int n3, int n4, aji aji2, int n5, int n6) {
        ags ags2 = new ags(n2, n3, n4, aji2, n5, n6);
        for (ags ags3 : this.S[this.T]) {
            if (!ags3.equals(ags2)) continue;
            return;
        }
        this.S[this.T].add(ags2);
    }

    private void Z() {
        while (!this.S[this.T].isEmpty()) {
            int n2 = this.T;
            this.T ^= 1;
            for (ags ags2 : this.S[n2]) {
                if (!this.a(ags2)) continue;
                this.J.ah().a(ags2.a(), ags2.b(), ags2.c(), 64.0, this.t.i, new gg(ags2.a(), ags2.b(), ags2.c(), ags2.f(), ags2.d(), ags2.e()));
            }
            this.S[n2].clear();
        }
    }

    private boolean a(ags ags2) {
        aji aji2 = this.a(ags2.a(), ags2.b(), ags2.c());
        if (aji2 == ags2.f()) {
            return aji2.a((ahb)this, ags2.a(), ags2.b(), ags2.c(), ags2.d(), ags2.e());
        }
        return false;
    }

    public void n() {
        this.w.a();
    }

    @Override
    protected void o() {
        boolean bl2 = this.Q();
        super.o();
        if (this.m != this.n) {
            this.J.ah().a(new gv(7, this.n), this.t.i);
        }
        if (this.o != this.p) {
            this.J.ah().a(new gv(8, this.p), this.t.i);
        }
        if (bl2 != this.Q()) {
            if (bl2) {
                this.J.ah().a(new gv(2, 0.0f));
            } else {
                this.J.ah().a(new gv(1, 0.0f));
            }
            this.J.ah().a(new gv(7, this.n));
            this.J.ah().a(new gv(8, this.p));
        }
    }

    @Override
    protected int p() {
        return this.J.ah().s();
    }

    public MinecraftServer q() {
        return this.J;
    }

    public mn r() {
        return this.K;
    }

    public mq t() {
        return this.L;
    }

    public ahp u() {
        return this.Q;
    }

    public void a(String string, double d2, double d3, double d4, int n2, double d5, double d6, double d7, double d8) {
        hb hb2 = new hb(string, (float)d2, (float)d3, (float)d4, (float)d5, (float)d6, (float)d7, (float)d8, n2);
        for (int i2 = 0; i2 < this.h.size(); ++i2) {
            mw mw2 = (mw)this.h.get(i2);
            r r2 = mw2.f_();
            double d9 = d2 - (double)r2.a;
            double d10 = d3 - (double)r2.b;
            double d11 = d4 - (double)r2.c;
            double d12 = d9 * d9 + d10 * d10 + d11 * d11;
            if (!(d12 <= 256.0)) continue;
            mw2.a.a(hb2);
        }
    }
}

