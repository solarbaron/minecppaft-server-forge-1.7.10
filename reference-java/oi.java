/*
 * Decompiled with CFR 0.152.
 */
import com.google.common.base.Charsets;
import com.google.common.collect.Lists;
import com.google.common.collect.Maps;
import com.mojang.authlib.GameProfile;
import java.io.File;
import java.net.SocketAddress;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.UUID;
import net.minecraft.server.MinecraftServer;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public abstract class oi {
    public static final File a = new File("banned-players.json");
    public static final File b = new File("banned-ips.json");
    public static final File c = new File("ops.json");
    public static final File d = new File("whitelist.json");
    private static final Logger g = LogManager.getLogger();
    private static final SimpleDateFormat h = new SimpleDateFormat("yyyy-MM-dd 'at' HH:mm:ss z");
    private final MinecraftServer i;
    public final List e = new ArrayList();
    private final op j = new op(a);
    private final nx k = new nx(b);
    private final oj l = new oj(c);
    private final or m = new or(d);
    private final Map n = Maps.newHashMap();
    private azp o;
    private boolean p;
    protected int f;
    private int q;
    private ahk r;
    private boolean s;
    private int t;

    public oi(MinecraftServer minecraftServer) {
        this.i = minecraftServer;
        this.j.a(false);
        this.k.a(false);
        this.f = 8;
    }

    public void a(ej ej2, mw mw2) {
        GameProfile gameProfile = mw2.bJ();
        ns ns2 = this.i.ax();
        GameProfile gameProfile2 = ns2.a(gameProfile.getId());
        String string = gameProfile2 == null ? gameProfile.getName() : gameProfile2.getName();
        ns2.a(gameProfile);
        dh dh2 = this.a(mw2);
        mw2.a(this.i.a(mw2.ap));
        mw2.c.a((mt)mw2.o);
        String string2 = "local";
        if (ej2.b() != null) {
            string2 = ej2.b().toString();
        }
        g.info(mw2.b_() + "[" + string2 + "] logged in with entity id " + mw2.y() + " at (" + mw2.s + ", " + mw2.t + ", " + mw2.u + ")");
        mt mt2 = this.i.a(mw2.ap);
        r r2 = mt2.K();
        this.a(mw2, null, mt2);
        nh nh2 = new nh(this.i, ej2, mw2);
        nh2.a(new hd(mw2.y(), mw2.c.b(), mt2.N().t(), mt2.t.i, mt2.r, this.p(), mt2.N().u()));
        nh2.a(new gr("MC|Brand", this.c().getServerModName().getBytes(Charsets.UTF_8)));
        nh2.a(new ig(r2.a, r2.b, r2.c));
        nh2.a(new hk(mw2.bE));
        nh2.a(new hu(mw2.bm.c));
        mw2.w().d();
        mw2.w().b(mw2);
        this.a((lp)mt2.W(), mw2);
        this.i.az();
        fr fr2 = !mw2.b_().equalsIgnoreCase(string) ? new fr("multiplayer.player.joined.renamed", mw2.c_(), string) : new fr("multiplayer.player.joined", mw2.c_());
        fr2.b().a(a.o);
        this.a(fr2);
        this.c(mw2);
        nh2.a(mw2.s, mw2.t, mw2.u, mw2.y, mw2.z);
        this.b(mw2, mt2);
        if (this.i.V().length() > 0) {
            mw2.a(this.i.V());
        }
        Object object = mw2.aQ().iterator();
        while (object.hasNext()) {
            rw rw2 = (rw)object.next();
            nh2.a(new in(mw2.y(), rw2));
        }
        mw2.d_();
        if (dh2 != null && dh2.b("Riding", 10) && (object = sg.a(dh2.m("Riding"), (ahb)mt2)) != null) {
            ((sa)object).n = true;
            mt2.d((sa)object);
            mw2.a((sa)object);
            ((sa)object).n = false;
        }
    }

    protected void a(lp lp2, mw mw2) {
        HashSet<Object> hashSet = new HashSet<Object>();
        for (Object object : lp2.g()) {
            mw2.a.a(new id((azy)object, 0));
        }
        for (int i2 = 0; i2 < 3; ++i2) {
            Object object;
            object = lp2.a(i2);
            if (object == null || hashSet.contains(object)) continue;
            List list = lp2.d((azx)object);
            for (ft ft2 : list) {
                mw2.a.a(ft2);
            }
            hashSet.add(object);
        }
    }

    public void a(mt[] mtArray) {
        this.o = mtArray[0].M().e();
    }

    public void a(mw mw2, mt mt2) {
        mt mt3 = mw2.r();
        if (mt2 != null) {
            mt2.t().c(mw2);
        }
        mt3.t().a(mw2);
        mt3.b.c((int)mw2.s >> 4, (int)mw2.u >> 4);
    }

    public int d() {
        return mq.b(this.s());
    }

    public dh a(mw mw2) {
        dh dh2;
        dh dh3 = this.i.c[0].N().i();
        if (mw2.b_().equals(this.i.M()) && dh3 != null) {
            mw2.f(dh3);
            dh2 = dh3;
            g.debug("loading single player");
        } else {
            dh2 = this.o.b(mw2);
        }
        return dh2;
    }

    protected void b(mw mw2) {
        this.o.a(mw2);
        pg pg2 = (pg)this.n.get(mw2.aB());
        if (pg2 != null) {
            pg2.b();
        }
    }

    public void c(mw mw2) {
        this.a(new ho(mw2.b_(), true, 1000));
        this.e.add(mw2);
        mt mt2 = this.i.a(mw2.ap);
        mt2.d(mw2);
        this.a(mw2, null);
        for (int i2 = 0; i2 < this.e.size(); ++i2) {
            mw mw3 = (mw)this.e.get(i2);
            mw2.a.a(new ho(mw3.b_(), true, mw3.h));
        }
    }

    public void d(mw mw2) {
        mw2.r().t().d(mw2);
    }

    public void e(mw mw2) {
        mw2.a(pp.f);
        this.b(mw2);
        mt mt2 = mw2.r();
        if (mw2.m != null) {
            mt2.f(mw2.m);
            g.debug("removing player mount");
        }
        mt2.e(mw2);
        mt2.t().c(mw2);
        this.e.remove(mw2);
        this.n.remove(mw2.aB());
        this.a(new ho(mw2.b_(), false, 9999));
    }

    public String a(SocketAddress socketAddress, GameProfile gameProfile) {
        if (this.j.a(gameProfile)) {
            oq oq2 = (oq)this.j.b((Object)gameProfile);
            String string = "You are banned from this server!\nReason: " + oq2.d();
            if (oq2.c() != null) {
                string = string + "\nYour ban will be removed on " + h.format(oq2.c());
            }
            return string;
        }
        if (!this.e(gameProfile)) {
            return "You are not white-listed on this server!";
        }
        if (this.k.a(socketAddress)) {
            ny ny2 = this.k.b(socketAddress);
            String string = "Your IP address is banned from this server!\nReason: " + ny2.d();
            if (ny2.c() != null) {
                string = string + "\nYour ban will be removed on " + h.format(ny2.c());
            }
            return string;
        }
        if (this.e.size() >= this.f) {
            return "The server is full!";
        }
        return null;
    }

    public mw f(GameProfile gameProfile) {
        UUID uUID = yz.a(gameProfile);
        ArrayList<mw> arrayList = Lists.newArrayList();
        for (int i2 = 0; i2 < this.e.size(); ++i2) {
            mw mw2 = (mw)this.e.get(i2);
            if (!mw2.aB().equals(uUID)) continue;
            arrayList.add(mw2);
        }
        for (mw mw2 : arrayList) {
            mw2.a.c("You logged in from another location");
        }
        mx mx2 = this.i.R() ? new ml(this.i.a(0)) : new mx(this.i.a(0));
        return new mw(this.i, this.i.a(0), gameProfile, mx2);
    }

    public mw a(mw mw2, int n2, boolean bl2) {
        r r2;
        mw2.r().r().a(mw2);
        mw2.r().r().b(mw2);
        mw2.r().t().c(mw2);
        this.e.remove(mw2);
        this.i.a(mw2.ap).f(mw2);
        r r3 = mw2.bN();
        boolean bl3 = mw2.bO();
        mw2.ap = n2;
        mx mx2 = this.i.R() ? new ml(this.i.a(mw2.ap)) : new mx(this.i.a(mw2.ap));
        mw mw3 = new mw(this.i, this.i.a(mw2.ap), mw2.bJ(), mx2);
        mw3.a = mw2.a;
        mw3.a(mw2, bl2);
        mw3.d(mw2.y());
        mt mt2 = this.i.a(mw2.ap);
        this.a(mw3, mw2, mt2);
        if (r3 != null) {
            r2 = yz.a(this.i.a(mw2.ap), r3, bl3);
            if (r2 != null) {
                mw3.b((float)r2.a + 0.5f, (float)r2.b + 0.1f, (float)r2.c + 0.5f, 0.0f, 0.0f);
                mw3.a(r3, bl3);
            } else {
                mw3.a.a(new gv(0, 0.0f));
            }
        }
        mt2.b.c((int)mw3.s >> 4, (int)mw3.u >> 4);
        while (!mt2.a((sa)mw3, mw3.C).isEmpty()) {
            mw3.b(mw3.s, mw3.t + 1.0, mw3.u);
        }
        mw3.a.a(new hs(mw3.ap, mw3.o.r, mw3.o.N().u(), mw3.c.b()));
        r2 = mt2.K();
        mw3.a.a(mw3.s, mw3.t, mw3.u, mw3.y, mw3.z);
        mw3.a.a(new ig(r2.a, r2.b, r2.c));
        mw3.a.a(new ia(mw3.bH, mw3.bG, mw3.bF));
        this.b(mw3, mt2);
        mt2.t().a(mw3);
        mt2.d(mw3);
        this.e.add(mw3);
        mw3.d_();
        mw3.g(mw3.aS());
        return mw3;
    }

    public void a(mw mw2, int n2) {
        int n3 = mw2.ap;
        mt mt2 = this.i.a(mw2.ap);
        mw2.ap = n2;
        mt mt3 = this.i.a(mw2.ap);
        mw2.a.a(new hs(mw2.ap, mw2.o.r, mw2.o.N().u(), mw2.c.b()));
        mt2.f(mw2);
        mw2.K = false;
        this.a(mw2, n3, mt2, mt3);
        this.a(mw2, mt2);
        mw2.a.a(mw2.s, mw2.t, mw2.u, mw2.y, mw2.z);
        mw2.c.a(mt3);
        this.b(mw2, mt3);
        this.f(mw2);
        for (rw rw2 : mw2.aQ()) {
            mw2.a.a(new in(mw2.y(), rw2));
        }
    }

    public void a(sa sa2, int n2, mt mt2, mt mt3) {
        double d2 = sa2.s;
        double d3 = sa2.u;
        double d4 = 8.0;
        double d5 = sa2.s;
        double d6 = sa2.t;
        double d7 = sa2.u;
        float f2 = sa2.y;
        mt2.C.a("moving");
        if (sa2.ap == -1) {
            sa2.b(d2 /= d4, sa2.t, d3 /= d4, sa2.y, sa2.z);
            if (sa2.Z()) {
                mt2.a(sa2, false);
            }
        } else if (sa2.ap == 0) {
            sa2.b(d2 *= d4, sa2.t, d3 *= d4, sa2.y, sa2.z);
            if (sa2.Z()) {
                mt2.a(sa2, false);
            }
        } else {
            r r2 = n2 == 1 ? mt3.K() : mt3.l();
            d2 = r2.a;
            sa2.t = r2.b;
            d3 = r2.c;
            sa2.b(d2, sa2.t, d3, 90.0f, 0.0f);
            if (sa2.Z()) {
                mt2.a(sa2, false);
            }
        }
        mt2.C.b();
        if (n2 != 1) {
            mt2.C.a("placing");
            d2 = qh.a((int)d2, -29999872, 29999872);
            d3 = qh.a((int)d3, -29999872, 29999872);
            if (sa2.Z()) {
                sa2.b(d2, sa2.t, d3, sa2.y, sa2.z);
                mt3.u().a(sa2, d5, d6, d7, f2);
                mt3.d(sa2);
                mt3.a(sa2, false);
            }
            mt2.C.b();
        }
        sa2.a(mt3);
    }

    public void e() {
        if (++this.t > 600) {
            this.t = 0;
        }
        if (this.t < this.e.size()) {
            mw mw2 = (mw)this.e.get(this.t);
            this.a(new ho(mw2.b_(), true, mw2.h));
        }
    }

    public void a(ft ft2) {
        for (int i2 = 0; i2 < this.e.size(); ++i2) {
            ((mw)this.e.get((int)i2)).a.a(ft2);
        }
    }

    public void a(ft ft2, int n2) {
        for (int i2 = 0; i2 < this.e.size(); ++i2) {
            mw mw2 = (mw)this.e.get(i2);
            if (mw2.ap != n2) continue;
            mw2.a.a(ft2);
        }
    }

    public String b(boolean bl2) {
        String string = "";
        ArrayList arrayList = Lists.newArrayList(this.e);
        for (int i2 = 0; i2 < arrayList.size(); ++i2) {
            if (i2 > 0) {
                string = string + ", ";
            }
            string = string + ((mw)arrayList.get(i2)).b_();
            if (!bl2) continue;
            string = string + " (" + ((mw)arrayList.get(i2)).aB().toString() + ")";
        }
        return string;
    }

    public String[] f() {
        String[] stringArray = new String[this.e.size()];
        for (int i2 = 0; i2 < this.e.size(); ++i2) {
            stringArray[i2] = ((mw)this.e.get(i2)).b_();
        }
        return stringArray;
    }

    public GameProfile[] g() {
        GameProfile[] gameProfileArray = new GameProfile[this.e.size()];
        for (int i2 = 0; i2 < this.e.size(); ++i2) {
            gameProfileArray[i2] = ((mw)this.e.get(i2)).bJ();
        }
        return gameProfileArray;
    }

    public op h() {
        return this.j;
    }

    public nx i() {
        return this.k;
    }

    public void a(GameProfile gameProfile) {
        this.l.a(new ok(gameProfile, this.i.l()));
    }

    public void b(GameProfile gameProfile) {
        this.l.c(gameProfile);
    }

    public boolean e(GameProfile gameProfile) {
        return !this.p || this.l.d(gameProfile) || this.m.d(gameProfile);
    }

    public boolean g(GameProfile gameProfile) {
        return this.l.d(gameProfile) || this.i.N() && this.i.c[0].N().v() && this.i.M().equalsIgnoreCase(gameProfile.getName()) || this.s;
    }

    public mw a(String string) {
        for (mw mw2 : this.e) {
            if (!mw2.b_().equalsIgnoreCase(string)) continue;
            return mw2;
        }
        return null;
    }

    public List a(r r2, int n2, int n3, int n4, int n5, int n6, int n7, Map map, String string, String string2, ahb ahb2) {
        if (this.e.isEmpty()) {
            return Collections.emptyList();
        }
        List<mw> list = new ArrayList();
        boolean bl2 = n4 < 0;
        boolean bl3 = string != null && string.startsWith("!");
        boolean bl4 = string2 != null && string2.startsWith("!");
        int n8 = n2 * n2;
        int n9 = n3 * n3;
        n4 = qh.a(n4);
        if (bl3) {
            string = string.substring(1);
        }
        if (bl4) {
            string2 = string2.substring(1);
        }
        for (int i2 = 0; i2 < this.e.size(); ++i2) {
            mw mw2 = (mw)this.e.get(i2);
            if (ahb2 != null && mw2.o != ahb2 || string != null && bl3 == string.equalsIgnoreCase(mw2.b_())) continue;
            if (string2 != null) {
                String string3;
                bae bae2 = mw2.bt();
                String string4 = string3 = bae2 == null ? "" : bae2.b();
                if (bl4 == string2.equalsIgnoreCase(string3)) continue;
            }
            if (r2 != null && (n2 > 0 || n3 > 0)) {
                float f2 = r2.e(mw2.f_());
                if (n2 > 0 && f2 < (float)n8 || n3 > 0 && f2 > (float)n9) continue;
            }
            if (!this.a((yz)mw2, map) || n5 != ahk.a.a() && n5 != mw2.c.b().a() || n6 > 0 && mw2.bF < n6 || mw2.bF > n7) continue;
            list.add(mw2);
        }
        if (r2 != null) {
            Collections.sort(list, new ln(r2));
        }
        if (bl2) {
            Collections.reverse(list);
        }
        if (n4 > 0) {
            list = list.subList(0, Math.min(n4, list.size()));
        }
        return list;
    }

    private boolean a(yz yz2, Map map) {
        if (map == null || map.size() == 0) {
            return true;
        }
        for (Map.Entry entry : map.entrySet()) {
            bac bac2;
            azx azx2;
            String string = (String)entry.getKey();
            boolean bl2 = false;
            if (string.endsWith("_min") && string.length() > 4) {
                bl2 = true;
                string = string.substring(0, string.length() - 4);
            }
            if ((azx2 = (bac2 = yz2.bU()).b(string)) == null) {
                return false;
            }
            azz azz2 = yz2.bU().a(yz2.b_(), azx2);
            int n2 = azz2.c();
            if (n2 < (Integer)entry.getValue() && bl2) {
                return false;
            }
            if (n2 <= (Integer)entry.getValue() || bl2) continue;
            return false;
        }
        return true;
    }

    public void a(double d2, double d3, double d4, double d5, int n2, ft ft2) {
        this.a(null, d2, d3, d4, d5, n2, ft2);
    }

    public void a(yz yz2, double d2, double d3, double d4, double d5, int n2, ft ft2) {
        for (int i2 = 0; i2 < this.e.size(); ++i2) {
            double d6;
            double d7;
            double d8;
            mw mw2 = (mw)this.e.get(i2);
            if (mw2 == yz2 || mw2.ap != n2 || !((d8 = d2 - mw2.s) * d8 + (d7 = d3 - mw2.t) * d7 + (d6 = d4 - mw2.u) * d6 < d5 * d5)) continue;
            mw2.a.a(ft2);
        }
    }

    public void j() {
        for (int i2 = 0; i2 < this.e.size(); ++i2) {
            this.b((mw)this.e.get(i2));
        }
    }

    public void d(GameProfile gameProfile) {
        this.m.a(new os(gameProfile));
    }

    public void c(GameProfile gameProfile) {
        this.m.c(gameProfile);
    }

    public or k() {
        return this.m;
    }

    public String[] l() {
        return this.m.a();
    }

    public oj m() {
        return this.l;
    }

    public String[] n() {
        return this.l.a();
    }

    public void a() {
    }

    public void b(mw mw2, mt mt2) {
        mw2.a.a(new ih(mt2.I(), mt2.J(), mt2.O().b("doDaylightCycle")));
        if (mt2.Q()) {
            mw2.a.a(new gv(1, 0.0f));
            mw2.a.a(new gv(7, mt2.j(1.0f)));
            mw2.a.a(new gv(8, mt2.h(1.0f)));
        }
    }

    public void f(mw mw2) {
        mw2.a(mw2.bn);
        mw2.o();
        mw2.a.a(new hu(mw2.bm.c));
    }

    public int o() {
        return this.e.size();
    }

    public int p() {
        return this.f;
    }

    public String[] q() {
        return this.i.c[0].M().e().f();
    }

    public boolean r() {
        return this.p;
    }

    public void a(boolean bl2) {
        this.p = bl2;
    }

    public List b(String string) {
        ArrayList<mw> arrayList = new ArrayList<mw>();
        for (mw mw2 : this.e) {
            if (!mw2.s().equals(string)) continue;
            arrayList.add(mw2);
        }
        return arrayList;
    }

    public int s() {
        return this.q;
    }

    public MinecraftServer c() {
        return this.i;
    }

    public dh t() {
        return null;
    }

    private void a(mw mw2, mw mw3, ahb ahb2) {
        if (mw3 != null) {
            mw2.c.a(mw3.c.b());
        } else if (this.r != null) {
            mw2.c.a(this.r);
        }
        mw2.c.b(ahb2.N().r());
    }

    public void u() {
        for (int i2 = 0; i2 < this.e.size(); ++i2) {
            ((mw)this.e.get((int)i2)).a.c("Server closed");
        }
    }

    public void a(fj fj2, boolean bl2) {
        this.i.a(fj2);
        this.a(new gj(fj2, bl2));
    }

    public void a(fj fj2) {
        this.a(fj2, true);
    }

    public pg a(yz yz2) {
        pg pg2;
        UUID uUID = yz2.aB();
        pg pg3 = pg2 = uUID == null ? null : (pg)this.n.get(uUID);
        if (pg2 == null) {
            File file;
            File file2 = new File(this.i.a(0).M().b(), "stats");
            File file3 = new File(file2, uUID.toString() + ".json");
            if (!file3.exists() && (file = new File(file2, yz2.b_() + ".json")).exists() && file.isFile()) {
                file.renameTo(file3);
            }
            pg2 = new pg(this.i, file3);
            pg2.a();
            this.n.put(uUID, pg2);
        }
        return pg2;
    }

    public void a(int n2) {
        this.q = n2;
        if (this.i.c == null) {
            return;
        }
        for (mt mt2 : this.i.c) {
            if (mt2 == null) continue;
            mt2.t().a(n2);
        }
    }
}

