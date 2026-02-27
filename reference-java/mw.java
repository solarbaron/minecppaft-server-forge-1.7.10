/*
 * Decompiled with CFR 0.152.
 */
import com.google.common.collect.Sets;
import com.mojang.authlib.GameProfile;
import io.netty.buffer.Unpooled;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import net.minecraft.server.MinecraftServer;
import org.apache.commons.io.Charsets;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class mw
extends yz
implements aac {
    private static final Logger bL = LogManager.getLogger();
    private String bM = "en_US";
    public nh a;
    public final MinecraftServer b;
    public final mx c;
    public double d;
    public double e;
    public final List f = new LinkedList();
    private final List bN = new LinkedList();
    private final pg bO;
    private float bP = Float.MIN_VALUE;
    private float bQ = -1.0E8f;
    private int bR = -99999999;
    private boolean bS = true;
    private int bT = -99999999;
    private int bU = 60;
    private zb bV;
    private boolean bW = true;
    private long bX = System.currentTimeMillis();
    private int bY;
    public boolean g;
    public int h;
    public boolean i;

    public mw(MinecraftServer minecraftServer, mt mt2, GameProfile gameProfile, mx mx2) {
        super(mt2, gameProfile);
        mx2.b = this;
        this.c = mx2;
        r r2 = mt2.K();
        int n2 = r2.a;
        int n3 = r2.c;
        int n4 = r2.b;
        if (!mt2.t.g && mt2.N().r() != ahk.d) {
            int n5 = Math.max(5, minecraftServer.ao() - 6);
            n4 = mt2.i(n2 += this.Z.nextInt(n5 * 2) - n5, n3 += this.Z.nextInt(n5 * 2) - n5);
        }
        this.b = minecraftServer;
        this.bO = minecraftServer.ah().a((yz)this);
        this.W = 0.0f;
        this.L = 0.0f;
        this.b((double)n2 + 0.5, n4, (double)n3 + 0.5, 0.0f, 0.0f);
        while (!mt2.a((sa)this, this.C).isEmpty()) {
            this.b(this.s, this.t + 1.0, this.u);
        }
    }

    @Override
    public void a(dh dh2) {
        super.a(dh2);
        if (dh2.b("playerGameType", 99)) {
            if (MinecraftServer.I().ap()) {
                this.c.a(MinecraftServer.I().i());
            } else {
                this.c.a(ahk.a(dh2.f("playerGameType")));
            }
        }
    }

    @Override
    public void b(dh dh2) {
        super.b(dh2);
        dh2.a("playerGameType", this.c.b().a());
    }

    @Override
    public void a(int n2) {
        super.a(n2);
        this.bT = -1;
    }

    public void d_() {
        this.bo.a(this);
    }

    @Override
    protected void e_() {
        this.L = 0.0f;
    }

    @Override
    public float g() {
        return 1.62f;
    }

    @Override
    public void h() {
        Object object;
        Object object2;
        this.c.a();
        --this.bU;
        if (this.ad > 0) {
            --this.ad;
        }
        this.bo.b();
        if (!this.o.E && !this.bo.a(this)) {
            this.k();
            this.bo = this.bn;
        }
        while (!this.bN.isEmpty()) {
            int n2 = Math.min(this.bN.size(), 127);
            object2 = new int[n2];
            object = this.bN.iterator();
            int n3 = 0;
            while (object.hasNext() && n3 < n2) {
                object2[n3++] = (Integer)object.next();
                object.remove();
            }
            this.a.a(new hq((int[])object2));
        }
        if (!this.f.isEmpty()) {
            Object object32;
            ArrayList<Object> arrayList = new ArrayList<Object>();
            object2 = this.f.iterator();
            object = new ArrayList();
            while (object2.hasNext() && arrayList.size() < gz.c()) {
                agu agu2 = (agu)object2.next();
                if (agu2 != null) {
                    if (!this.o.d(agu2.a << 4, 0, agu2.b << 4) || !((apx)(object32 = this.o.e(agu2.a, agu2.b))).k()) continue;
                    arrayList.add(object32);
                    object.addAll(((mt)this.o).a(agu2.a * 16, 0, agu2.b * 16, agu2.a * 16 + 16, 256, agu2.b * 16 + 16));
                    object2.remove();
                    continue;
                }
                object2.remove();
            }
            if (!arrayList.isEmpty()) {
                this.a.a(new gz(arrayList));
                Iterator iterator = object.iterator();
                while (iterator.hasNext()) {
                    object32 = (aor)iterator.next();
                    this.b((aor)object32);
                }
                for (Object object32 : arrayList) {
                    this.r().r().a(this, (apx)object32);
                }
            }
        }
    }

    public void i() {
        try {
            super.h();
            for (int i2 = 0; i2 < this.bm.a(); ++i2) {
                Object object;
                add add2 = this.bm.a(i2);
                if (add2 == null || !add2.b().h() || (object = ((abs)add2.b()).c(add2, this.o, this)) == null) continue;
                this.a.a((ft)object);
            }
            if (this.aS() != this.bQ || this.bR != this.bp.a() || this.bp.e() == 0.0f != this.bS) {
                this.a.a(new ib(this.aS(), this.bp.a(), this.bp.e()));
                this.bQ = this.aS();
                this.bR = this.bp.a();
                boolean bl2 = this.bS = this.bp.e() == 0.0f;
            }
            if (this.aS() + this.bs() != this.bP) {
                this.bP = this.aS() + this.bs();
                Collection collection = this.bU().a(bah.f);
                for (Object object : collection) {
                    this.bU().a(this.b_(), (azx)object).a(Arrays.asList(this));
                }
            }
            if (this.bG != this.bT) {
                this.bT = this.bG;
                this.a.a(new ia(this.bH, this.bG, this.bF));
            }
            if (this.aa % 20 * 5 == 0 && !this.w().a(pc.L)) {
                this.j();
            }
        }
        catch (Throwable throwable) {
            b b2 = b.a(throwable, "Ticking player");
            k k2 = b2.a("Player being ticked");
            this.a(k2);
            throw new s(b2);
        }
    }

    protected void j() {
        ahu ahu2 = this.o.a(qh.c(this.s), qh.c(this.u));
        if (ahu2 != null) {
            String string = ahu2.af;
            pr pr2 = (pr)this.w().b((ph)pc.L);
            if (pr2 == null) {
                pr2 = (pr)this.w().a(pc.L, new pr());
            }
            pr2.add(string);
            if (this.w().b(pc.L) && pr2.size() == ahu.n.size()) {
                HashSet hashSet = Sets.newHashSet(ahu.n);
                for (String string2 : pr2) {
                    Iterator iterator = hashSet.iterator();
                    while (iterator.hasNext()) {
                        ahu ahu3 = (ahu)iterator.next();
                        if (!ahu3.af.equals(string2)) continue;
                        iterator.remove();
                    }
                    if (!hashSet.isEmpty()) continue;
                    break;
                }
                if (hashSet.isEmpty()) {
                    this.a(pc.L);
                }
            }
        }
    }

    @Override
    public void a(ro ro2) {
        Object object;
        this.b.ah().a(this.aW().b());
        if (!this.o.O().b("keepInventory")) {
            this.bm.m();
        }
        Collection collection = this.o.W().a(bah.c);
        for (azx azx2 : collection) {
            object = this.bU().a(this.b_(), azx2);
            ((azz)object).a();
        }
        sv sv2 = this.aX();
        if (sv2 != null) {
            int n2 = sg.a(sv2);
            object = (sh)sg.a.get(n2);
            if (object != null) {
                this.a(((sh)object).e, 1);
            }
            sv2.b(this, this.ba);
        }
        this.a(pp.v, 1);
        this.aW().g();
    }

    @Override
    public boolean a(ro ro2, float f2) {
        boolean bl2;
        if (this.aw()) {
            return false;
        }
        boolean bl3 = bl2 = this.b.X() && this.b.ab() && "fall".equals(ro2.o);
        if (!bl2 && this.bU > 0 && ro2 != ro.i) {
            return false;
        }
        if (ro2 instanceof rp) {
            sa sa2 = ro2.j();
            if (sa2 instanceof yz && !this.a((yz)sa2)) {
                return false;
            }
            if (sa2 instanceof zc) {
                zc zc2 = (zc)sa2;
                if (zc2.c instanceof yz && !this.a((yz)zc2.c)) {
                    return false;
                }
            }
        }
        return super.a(ro2, f2);
    }

    @Override
    public boolean a(yz yz2) {
        if (!this.b.ab()) {
            return false;
        }
        return super.a(yz2);
    }

    @Override
    public void b(int n2) {
        if (this.ap == 1 && n2 == 1) {
            this.a(pc.D);
            this.o.e(this);
            this.i = true;
            this.a.a(new gv(4, 0.0f));
        } else {
            if (this.ap == 0 && n2 == 1) {
                this.a(pc.C);
                r r2 = this.b.a(n2).l();
                if (r2 != null) {
                    this.a.a(r2.a, r2.b, r2.c, 0.0f, 0.0f);
                }
                n2 = 1;
            } else {
                this.a(pc.y);
            }
            this.b.ah().a(this, n2);
            this.bT = -1;
            this.bQ = -1.0f;
            this.bR = -1;
        }
    }

    private void b(aor aor2) {
        ft ft2;
        if (aor2 != null && (ft2 = aor2.m()) != null) {
            this.a.a(ft2);
        }
    }

    @Override
    public void a(sa sa2, int n2) {
        super.a(sa2, n2);
        this.bo.b();
    }

    @Override
    public za a(int n2, int n3, int n4) {
        za za2 = super.a(n2, n3, n4);
        if (za2 == za.a) {
            hp hp2 = new hp(this, n2, n3, n4);
            this.r().r().a((sa)this, hp2);
            this.a.a(this.s, this.t, this.u, this.y, this.z);
            this.a.a(hp2);
        }
        return za2;
    }

    @Override
    public void a(boolean bl2, boolean bl3, boolean bl4) {
        if (this.bm()) {
            this.r().r().b(this, new gc(this, 2));
        }
        super.a(bl2, bl3, bl4);
        if (this.a != null) {
            this.a.a(this.s, this.t, this.u, this.y, this.z);
        }
    }

    @Override
    public void a(sa sa2) {
        super.a(sa2);
        this.a.a(new hx(0, this, this.m));
        this.a.a(this.s, this.t, this.u, this.y, this.z);
    }

    @Override
    protected void a(double d2, boolean bl2) {
    }

    public void b(double d2, boolean bl2) {
        super.a(d2, bl2);
    }

    @Override
    public void a(aor aor2) {
        if (aor2 instanceof apm) {
            ((apm)aor2).a(this);
            this.a.a(new hj(aor2.c, aor2.d, aor2.e));
        }
    }

    private void bV() {
        this.bY = this.bY % 100 + 1;
    }

    @Override
    public void b(int n2, int n3, int n4) {
        this.bV();
        this.a.a(new gn(this.bY, 1, "Crafting", 9, true));
        this.bo = new aaf(this.bm, this.o, n2, n3, n4);
        this.bo.d = this.bY;
        this.bo.a(this);
    }

    @Override
    public void a(int n2, int n3, int n4, String string) {
        this.bV();
        this.a.a(new gn(this.bY, 4, string == null ? "" : string, 9, string != null));
        this.bo = new aag(this.bm, this.o, n2, n3, n4);
        this.bo.d = this.bY;
        this.bo.a(this);
    }

    @Override
    public void c(int n2, int n3, int n4) {
        this.bV();
        this.a.a(new gn(this.bY, 8, "Repairing", 9, true));
        this.bo = new zu(this.bm, this.o, n2, n3, n4, this);
        this.bo.d = this.bY;
        this.bo.a(this);
    }

    @Override
    public void a(rb rb2) {
        if (this.bo != this.bn) {
            this.k();
        }
        this.bV();
        this.a.a(new gn(this.bY, 0, rb2.b(), rb2.a(), rb2.k_()));
        this.bo = new aad(this.bm, rb2);
        this.bo.d = this.bY;
        this.bo.a(this);
    }

    @Override
    public void a(api api2) {
        this.bV();
        this.a.a(new gn(this.bY, 9, api2.b(), api2.a(), api2.k_()));
        this.bo = new aal(this.bm, api2);
        this.bo.d = this.bY;
        this.bo.a(this);
    }

    @Override
    public void a(xr xr2) {
        this.bV();
        this.a.a(new gn(this.bY, 9, xr2.b(), xr2.a(), xr2.k_()));
        this.bo = new aal(this.bm, xr2);
        this.bo.d = this.bY;
        this.bo.a(this);
    }

    @Override
    public void a(apg apg2) {
        this.bV();
        this.a.a(new gn(this.bY, 2, apg2.b(), apg2.a(), apg2.k_()));
        this.bo = new aaj(this.bm, apg2);
        this.bo.d = this.bY;
        this.bo.a(this);
    }

    @Override
    public void a(apb apb2) {
        this.bV();
        this.a.a(new gn(this.bY, apb2 instanceof apc ? 10 : 3, apb2.b(), apb2.a(), apb2.k_()));
        this.bo = new aaz(this.bm, apb2);
        this.bo.d = this.bY;
        this.bo.a(this);
    }

    @Override
    public void a(aov aov2) {
        this.bV();
        this.a.a(new gn(this.bY, 5, aov2.b(), aov2.a(), aov2.k_()));
        this.bo = new zz(this.bm, aov2);
        this.bo.d = this.bY;
        this.bo.a(this);
    }

    @Override
    public void a(aoq aoq2) {
        this.bV();
        this.a.a(new gn(this.bY, 7, aoq2.b(), aoq2.a(), aoq2.k_()));
        this.bo = new zx(this.bm, aoq2);
        this.bo.d = this.bY;
        this.bo.a(this);
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    @Override
    public void a(agm agm2, String string) {
        this.bV();
        this.bo = new aat(this.bm, agm2, this.o);
        this.bo.d = this.bY;
        this.bo.a(this);
        aas aas2 = ((aat)this.bo).e();
        this.a.a(new gn(this.bY, 6, string == null ? "" : string, aas2.a(), string != null));
        ago ago2 = agm2.b(this);
        if (ago2 != null) {
            et et2 = new et(Unpooled.buffer());
            try {
                et2.writeInt(this.bY);
                ago2.a(et2);
                this.a.a(new gr("MC|TrList", et2));
            }
            catch (IOException iOException) {
                bL.error("Couldn't send trade list", (Throwable)iOException);
            }
            finally {
                et2.release();
            }
        }
    }

    @Override
    public void a(wi wi2, rb rb2) {
        if (this.bo != this.bn) {
            this.k();
        }
        this.bV();
        this.a.a(new gn(this.bY, 11, rb2.b(), rb2.a(), rb2.k_(), wi2.y()));
        this.bo = new aam(this.bm, rb2, wi2);
        this.bo.d = this.bY;
        this.bo.a(this);
    }

    @Override
    public void a(zs zs2, int n2, add add2) {
        if (zs2.a(n2) instanceof aax) {
            return;
        }
        if (this.g) {
            return;
        }
        this.a.a(new gq(zs2.d, n2, add2));
    }

    public void a(zs zs2) {
        this.a(zs2, zs2.a());
    }

    @Override
    public void a(zs zs2, List list) {
        this.a.a(new go(zs2.d, list));
        this.a.a(new gq(-1, -1, this.bm.o()));
    }

    @Override
    public void a(zs zs2, int n2, int n3) {
        this.a.a(new gp(zs2.d, n2, n3));
    }

    @Override
    public void k() {
        this.a.a(new gm(this.bo.d));
        this.m();
    }

    public void l() {
        if (this.g) {
            return;
        }
        this.a.a(new gq(-1, -1, this.bm.o()));
    }

    public void m() {
        this.bo.b(this);
        this.bo = this.bn;
    }

    public void a(float f2, float f3, boolean bl2, boolean bl3) {
        if (this.m != null) {
            if (f2 >= -1.0f && f2 <= 1.0f) {
                this.bd = f2;
            }
            if (f3 >= -1.0f && f3 <= 1.0f) {
                this.be = f3;
            }
            this.bc = bl2;
            this.b(bl3);
        }
    }

    @Override
    public void a(ph ph2, int n2) {
        if (ph2 == null) {
            return;
        }
        this.bO.b(this, ph2, n2);
        for (azx azx2 : this.bU().a(ph2.k())) {
            this.bU().a(this.b_(), azx2).a();
        }
        if (this.bO.e()) {
            this.bO.a(this);
        }
    }

    public void n() {
        if (this.l != null) {
            this.l.a((sa)this);
        }
        if (this.bA) {
            this.a(true, false, false);
        }
    }

    public void o() {
        this.bQ = -1.0E8f;
    }

    @Override
    public void b(fj fj2) {
        this.a.a(new gj(fj2));
    }

    @Override
    protected void p() {
        this.a.a(new gt(this, 9));
        super.p();
    }

    @Override
    public void a(add add2, int n2) {
        super.a(add2, n2);
        if (add2 != null && add2.b() != null && add2.b().d(add2) == aei.b) {
            this.r().r().b(this, new gc(this, 3));
        }
    }

    @Override
    public void a(yz yz2, boolean bl2) {
        super.a(yz2, bl2);
        this.bT = -1;
        this.bQ = -1.0f;
        this.bR = -1;
        this.bN.addAll(((mw)yz2).bN);
    }

    @Override
    protected void a(rw rw2) {
        super.a(rw2);
        this.a.a(new in(this.y(), rw2));
    }

    @Override
    protected void a(rw rw2, boolean bl2) {
        super.a(rw2, bl2);
        this.a.a(new in(this.y(), rw2));
    }

    @Override
    protected void b(rw rw2) {
        super.b(rw2);
        this.a.a(new hr(this.y(), rw2));
    }

    @Override
    public void a(double d2, double d3, double d4) {
        this.a.a(d2, d3, d4, this.y, this.z);
    }

    @Override
    public void b(sa sa2) {
        this.r().r().b(this, new gc(sa2, 4));
    }

    @Override
    public void c(sa sa2) {
        this.r().r().b(this, new gc(sa2, 5));
    }

    @Override
    public void q() {
        if (this.a == null) {
            return;
        }
        this.a.a(new hk(this.bE));
    }

    public mt r() {
        return (mt)this.o;
    }

    @Override
    public void a(ahk ahk2) {
        this.c.a(ahk2);
        this.a.a(new gv(3, ahk2.a()));
    }

    @Override
    public void a(fj fj2) {
        this.a.a(new gj(fj2));
    }

    @Override
    public boolean a(int n2, String string) {
        if ("seed".equals(string) && !this.b.X()) {
            return true;
        }
        if ("tell".equals(string) || "help".equals(string) || "me".equals(string)) {
            return true;
        }
        if (this.b.ah().g(this.bJ())) {
            ok ok2 = (ok)this.b.ah().m().b((Object)this.bJ());
            if (ok2 != null) {
                return ok2.a() >= n2;
            }
            return this.b.l() >= n2;
        }
        return false;
    }

    public String s() {
        String string = this.a.a.b().toString();
        string = string.substring(string.indexOf("/") + 1);
        string = string.substring(0, string.indexOf(":"));
        return string;
    }

    public void a(iu iu2) {
        this.bM = iu2.c();
        int n2 = 256 >> iu2.d();
        if (n2 <= 3 || n2 < 20) {
            // empty if block
        }
        this.bV = iu2.e();
        this.bW = iu2.f();
        if (this.b.N() && this.b.M().equals(this.b_())) {
            this.b.a(iu2.g());
        }
        this.b(1, !iu2.h());
    }

    public zb u() {
        return this.bV;
    }

    public void a(String string) {
        this.a.a(new gr("MC|RPack", string.getBytes(Charsets.UTF_8)));
    }

    @Override
    public r f_() {
        return new r(qh.c(this.s), qh.c(this.t + 0.5), qh.c(this.u));
    }

    public void v() {
        this.bX = MinecraftServer.ar();
    }

    public pg w() {
        return this.bO;
    }

    public void d(sa sa2) {
        if (sa2 instanceof yz) {
            this.a.a(new hq(sa2.y()));
        } else {
            this.bN.add(sa2.y());
        }
    }

    public long x() {
        return this.bX;
    }
}

