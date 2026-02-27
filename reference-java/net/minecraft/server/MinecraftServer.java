/*
 * Decompiled with CFR 0.152.
 */
package net.minecraft.server;

import com.google.common.base.Charsets;
import com.mojang.authlib.GameProfile;
import com.mojang.authlib.GameProfileRepository;
import com.mojang.authlib.minecraft.MinecraftSessionService;
import com.mojang.authlib.yggdrasil.YggdrasilAuthenticationService;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufOutputStream;
import io.netty.buffer.Unpooled;
import io.netty.handler.codec.base64.Base64;
import java.awt.GraphicsEnvironment;
import java.awt.image.BufferedImage;
import java.awt.image.RenderedImage;
import java.io.File;
import java.net.Proxy;
import java.security.KeyPair;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Date;
import java.util.List;
import java.util.Random;
import java.util.UUID;
import javax.imageio.ImageIO;
import org.apache.commons.lang3.Validate;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public abstract class MinecraftServer
implements ac,
Runnable,
rk {
    private static final Logger i = LogManager.getLogger();
    public static final File a = new File("usercache.json");
    private static MinecraftServer j;
    private final aze k;
    private final ri l = new ri("server", this, MinecraftServer.ar());
    private final File m;
    private final List n = new ArrayList();
    private final ab o;
    public final qi b = new qi();
    private final nc p;
    private final kb q = new kb();
    private final Random r = new Random();
    private String s;
    private int t = -1;
    public mt[] c;
    private oi u;
    private boolean v = true;
    private boolean w;
    private int x;
    protected final Proxy d;
    public String e;
    public int f;
    private boolean y;
    private boolean z;
    private boolean A;
    private boolean B;
    private boolean C;
    private String D;
    private int E;
    private int F = 0;
    public final long[] g = new long[100];
    public long[][] h;
    private KeyPair G;
    private String H;
    private String I;
    private boolean K;
    private boolean L;
    private boolean M;
    private String N = "";
    private boolean O;
    private long P;
    private String Q;
    private boolean R;
    private boolean S;
    private final YggdrasilAuthenticationService T;
    private final MinecraftSessionService U;
    private long V = 0L;
    private final GameProfileRepository W;
    private final ns X = new ns(this, a);

    public MinecraftServer(File file, Proxy proxy) {
        j = this;
        this.d = proxy;
        this.m = file;
        this.p = new nc(this);
        this.o = new bl();
        this.k = new ayn(file);
        this.T = new YggdrasilAuthenticationService(proxy, UUID.randomUUID().toString());
        this.U = this.T.createMinecraftSessionService();
        this.W = this.T.createProfileRepository();
    }

    protected abstract boolean e();

    protected void a(String string) {
        if (this.S().b(string)) {
            i.info("Converting map!");
            this.b("menu.convertingLevel");
            this.S().a(string, new lh(this));
        }
    }

    protected synchronized void b(String string) {
        this.Q = string;
    }

    protected void a(String string, String string2, long l2, ahm ahm2, String string3) {
        ahj ahj2;
        this.a(string);
        this.b("menu.loadingLevel");
        this.c = new mt[3];
        this.h = new long[this.c.length][100];
        azc azc2 = this.k.a(string, true);
        ays ays2 = azc2.d();
        if (ays2 == null) {
            ahj2 = new ahj(l2, this.i(), this.h(), this.k(), ahm2);
            ahj2.a(string3);
        } else {
            ahj2 = new ahj(ays2);
        }
        if (this.L) {
            ahj2.a();
        }
        for (int i2 = 0; i2 < this.c.length; ++i2) {
            int n2 = 0;
            if (i2 == 1) {
                n2 = -1;
            }
            if (i2 == 2) {
                n2 = 1;
            }
            this.c[i2] = i2 == 0 ? (this.R() ? new mk(this, azc2, string2, n2, this.b) : new mt(this, azc2, string2, n2, ahj2, this.b)) : new mm(this, azc2, string2, n2, ahj2, this.c[0], this.b);
            this.c[i2].a(new mp(this, this.c[i2]));
            if (!this.N()) {
                this.c[i2].N().a(this.i());
            }
            this.u.a(this.c);
        }
        this.a(this.j());
        this.g();
    }

    protected void g() {
        int n2 = 16;
        int n3 = 4;
        int n4 = 192;
        int n5 = 625;
        int n6 = 0;
        this.b("menu.generatingTerrain");
        int n7 = 0;
        i.info("Preparing start region for level " + n7);
        mt mt2 = this.c[n7];
        r r2 = mt2.K();
        long l2 = MinecraftServer.ar();
        for (int i2 = -192; i2 <= 192 && this.q(); i2 += 16) {
            for (int i3 = -192; i3 <= 192 && this.q(); i3 += 16) {
                long l3 = MinecraftServer.ar();
                if (l3 - l2 > 1000L) {
                    this.a_("Preparing spawn area", n6 * 100 / 625);
                    l2 = l3;
                }
                ++n6;
                mt2.b.c(r2.a + i2 >> 4, r2.c + i3 >> 4);
            }
        }
        this.n();
    }

    public abstract boolean h();

    public abstract ahk i();

    public abstract rd j();

    public abstract boolean k();

    public abstract int l();

    public abstract boolean m();

    protected void a_(String string, int n2) {
        this.e = string;
        this.f = n2;
        i.info(string + ": " + n2 + "%");
    }

    protected void n() {
        this.e = null;
        this.f = 0;
    }

    protected void a(boolean bl2) {
        if (this.M) {
            return;
        }
        for (mt mt2 : this.c) {
            if (mt2 == null) continue;
            if (!bl2) {
                i.info("Saving chunks for level '" + mt2.N().k() + "'/" + mt2.t.l());
            }
            try {
                mt2.a(true, null);
            }
            catch (ahg ahg2) {
                i.warn(ahg2.getMessage());
            }
        }
    }

    public void o() {
        if (this.M) {
            return;
        }
        i.info("Stopping server");
        if (this.ai() != null) {
            this.ai().b();
        }
        if (this.u != null) {
            i.info("Saving players");
            this.u.j();
            this.u.u();
        }
        if (this.c != null) {
            i.info("Saving worlds");
            this.a(false);
            for (int i2 = 0; i2 < this.c.length; ++i2) {
                mt mt2 = this.c[i2];
                mt2.n();
            }
        }
        if (this.l.d()) {
            this.l.e();
        }
    }

    public String p() {
        return this.s;
    }

    public void c(String string) {
        this.s = string;
    }

    public boolean q() {
        return this.v;
    }

    public void r() {
        this.v = false;
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    @Override
    public void run() {
        try {
            if (this.e()) {
                long l2 = MinecraftServer.ar();
                long l3 = 0L;
                this.q.a(new fq(this.D));
                this.q.a(new kf("1.7.10", 5));
                this.a(this.q);
                while (this.v) {
                    long l4 = MinecraftServer.ar();
                    long l5 = l4 - l2;
                    if (l5 > 2000L && l2 - this.P >= 15000L) {
                        i.warn("Can't keep up! Did the system time change, or is the server overloaded? Running {}ms behind, skipping {} tick(s)", l5, l5 / 50L);
                        l5 = 2000L;
                        this.P = l2;
                    }
                    if (l5 < 0L) {
                        i.warn("Time ran backwards! Did the system time change?");
                        l5 = 0L;
                    }
                    l3 += l5;
                    l2 = l4;
                    if (this.c[0].e()) {
                        this.u();
                        l3 = 0L;
                    } else {
                        while (l3 > 50L) {
                            l3 -= 50L;
                            this.u();
                        }
                    }
                    Thread.sleep(Math.max(1L, 50L - l3));
                    this.O = true;
                }
            } else {
                this.a((b)null);
            }
        }
        catch (Throwable throwable) {
            i.error("Encountered an unexpected exception", throwable);
            b b2 = null;
            b2 = throwable instanceof s ? this.b(((s)throwable).a()) : this.b(new b("Exception in server tick loop", throwable));
            File file = new File(new File(this.s(), "crash-reports"), "crash-" + new SimpleDateFormat("yyyy-MM-dd_HH.mm.ss").format(new Date()) + "-server.txt");
            if (b2.a(file)) {
                i.error("This crash report has been saved to: " + file.getAbsolutePath());
            } else {
                i.error("We were unable to save this crash report to disk.");
            }
            this.a(b2);
        }
        finally {
            try {
                this.o();
                this.w = true;
            }
            catch (Throwable throwable) {
                i.error("Exception stopping the server", throwable);
            }
            finally {
                this.t();
            }
        }
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    private void a(kb kb2) {
        File file = this.d("server-icon.png");
        if (file.isFile()) {
            ByteBuf byteBuf = Unpooled.buffer();
            try {
                BufferedImage bufferedImage = ImageIO.read(file);
                Validate.validState(bufferedImage.getWidth() == 64, "Must be 64 pixels wide", new Object[0]);
                Validate.validState(bufferedImage.getHeight() == 64, "Must be 64 pixels high", new Object[0]);
                ImageIO.write((RenderedImage)bufferedImage, "PNG", new ByteBufOutputStream(byteBuf));
                ByteBuf byteBuf2 = Base64.encode(byteBuf);
                kb2.a("data:image/png;base64," + byteBuf2.toString(Charsets.UTF_8));
            }
            catch (Exception exception) {
                i.error("Couldn't load server icon", (Throwable)exception);
            }
            finally {
                byteBuf.release();
            }
        }
    }

    protected File s() {
        return new File(".");
    }

    protected void a(b b2) {
    }

    protected void t() {
    }

    protected void u() {
        long l2 = System.nanoTime();
        ++this.x;
        if (this.R) {
            this.R = false;
            this.b.a = true;
            this.b.a();
        }
        this.b.a("root");
        this.v();
        if (l2 - this.V >= 5000000000L) {
            this.V = l2;
            this.q.a(new kc(this.D(), this.C()));
            GameProfile[] gameProfileArray = new GameProfile[Math.min(this.C(), 12)];
            int n2 = qh.a(this.r, 0, this.C() - gameProfileArray.length);
            for (int i2 = 0; i2 < gameProfileArray.length; ++i2) {
                gameProfileArray[i2] = ((mw)this.u.e.get(n2 + i2)).bJ();
            }
            Collections.shuffle(Arrays.asList(gameProfileArray));
            this.q.b().a(gameProfileArray);
        }
        if (this.x % 900 == 0) {
            this.b.a("save");
            this.u.j();
            this.a(true);
            this.b.b();
        }
        this.b.a("tallying");
        this.g[this.x % 100] = System.nanoTime() - l2;
        this.b.b();
        this.b.a("snooper");
        if (!this.l.d() && this.x > 100) {
            this.l.a();
        }
        if (this.x % 6000 == 0) {
            this.l.b();
        }
        this.b.b();
        this.b.b();
    }

    public void v() {
        int n2;
        this.b.a("levels");
        for (n2 = 0; n2 < this.c.length; ++n2) {
            long l2 = System.nanoTime();
            if (n2 == 0 || this.w()) {
                mt mt2 = this.c[n2];
                this.b.a(mt2.N().k());
                this.b.a("pools");
                this.b.b();
                if (this.x % 20 == 0) {
                    this.b.a("timeSync");
                    this.u.a(new ih(mt2.I(), mt2.J(), mt2.O().b("doDaylightCycle")), mt2.t.i);
                    this.b.b();
                }
                this.b.a("tick");
                try {
                    mt2.b();
                }
                catch (Throwable throwable) {
                    b b2 = b.a(throwable, "Exception ticking world");
                    mt2.a(b2);
                    throw new s(b2);
                }
                try {
                    mt2.h();
                }
                catch (Throwable throwable) {
                    b b3 = b.a(throwable, "Exception ticking world entities");
                    mt2.a(b3);
                    throw new s(b3);
                }
                this.b.b();
                this.b.a("tracker");
                mt2.r().a();
                this.b.b();
                this.b.b();
            }
            this.h[n2][this.x % 100] = System.nanoTime() - l2;
        }
        this.b.c("connection");
        this.ai().c();
        this.b.c("players");
        this.u.e();
        this.b.c("tickables");
        for (n2 = 0; n2 < this.n.size(); ++n2) {
            ((lr)this.n.get(n2)).a();
        }
        this.b.b();
    }

    public boolean w() {
        return true;
    }

    public void a(lr lr2) {
        this.n.add(lr2);
    }

    public static void main(String[] stringArray) {
        kl.b();
        try {
            boolean bl2 = true;
            String string = null;
            String string2 = ".";
            String string3 = null;
            boolean bl3 = false;
            boolean bl4 = false;
            int n2 = -1;
            for (int i2 = 0; i2 < stringArray.length; ++i2) {
                String string4 = stringArray[i2];
                String string5 = i2 == stringArray.length - 1 ? null : stringArray[i2 + 1];
                boolean bl5 = false;
                if (string4.equals("nogui") || string4.equals("--nogui")) {
                    bl2 = false;
                } else if (string4.equals("--port") && string5 != null) {
                    bl5 = true;
                    try {
                        n2 = Integer.parseInt(string5);
                    }
                    catch (NumberFormatException numberFormatException) {}
                } else if (string4.equals("--singleplayer") && string5 != null) {
                    bl5 = true;
                    string = string5;
                } else if (string4.equals("--universe") && string5 != null) {
                    bl5 = true;
                    string2 = string5;
                } else if (string4.equals("--world") && string5 != null) {
                    bl5 = true;
                    string3 = string5;
                } else if (string4.equals("--demo")) {
                    bl3 = true;
                } else if (string4.equals("--bonusChest")) {
                    bl4 = true;
                }
                if (!bl5) continue;
                ++i2;
            }
            lt lt2 = new lt(new File(string2));
            if (string != null) {
                lt2.j(string);
            }
            if (string3 != null) {
                lt2.k(string3);
            }
            if (n2 >= 0) {
                lt2.b(n2);
            }
            if (bl3) {
                lt2.b(true);
            }
            if (bl4) {
                lt2.c(true);
            }
            if (bl2 && !GraphicsEnvironment.isHeadless()) {
                lt2.aD();
            }
            lt2.x();
            Runtime.getRuntime().addShutdownHook(new li("Server Shutdown Thread", lt2));
        }
        catch (Exception exception) {
            i.fatal("Failed to start the minecraft server", (Throwable)exception);
        }
    }

    public void x() {
        new lj(this, "Server thread").start();
    }

    public File d(String string) {
        return new File(this.s(), string);
    }

    public void e(String string) {
        i.info(string);
    }

    public void f(String string) {
        i.warn(string);
    }

    public mt a(int n2) {
        if (n2 == -1) {
            return this.c[1];
        }
        if (n2 == 1) {
            return this.c[2];
        }
        return this.c[0];
    }

    public String y() {
        return this.s;
    }

    public int z() {
        return this.t;
    }

    public String A() {
        return this.D;
    }

    public String B() {
        return "1.7.10";
    }

    public int C() {
        return this.u.o();
    }

    public int D() {
        return this.u.p();
    }

    public String[] E() {
        return this.u.f();
    }

    public GameProfile[] F() {
        return this.u.g();
    }

    public String G() {
        return "";
    }

    public String g(String string) {
        ov.a.e();
        this.o.a(ov.a, string);
        return ov.a.f();
    }

    public boolean H() {
        return false;
    }

    public void h(String string) {
        i.error(string);
    }

    public void i(String string) {
        if (this.H()) {
            i.info(string);
        }
    }

    public String getServerModName() {
        return "vanilla";
    }

    public b b(b b2) {
        b2.g().a("Profiler Position", new lk(this));
        if (this.c != null && this.c.length > 0 && this.c[0] != null) {
            b2.g().a("Vec3 Pool Size", new ll(this));
        }
        if (this.u != null) {
            b2.g().a("Player Count", new lm(this));
        }
        return b2;
    }

    public List a(ac ac2, String string) {
        ArrayList<String> arrayList = new ArrayList<String>();
        if (string.startsWith("/")) {
            boolean bl2 = !(string = string.substring(1)).contains(" ");
            List list = this.o.b(ac2, string);
            if (list != null) {
                for (String string2 : list) {
                    if (bl2) {
                        arrayList.add("/" + string2);
                        continue;
                    }
                    arrayList.add(string2);
                }
            }
            return arrayList;
        }
        String[] stringArray = string.split(" ", -1);
        String string3 = stringArray[stringArray.length - 1];
        for (String string4 : this.u.f()) {
            if (!y.a(string3, string4)) continue;
            arrayList.add(string4);
        }
        return arrayList;
    }

    public static MinecraftServer I() {
        return j;
    }

    @Override
    public String b_() {
        return "Server";
    }

    @Override
    public void a(fj fj2) {
        i.info(fj2.c());
    }

    @Override
    public boolean a(int n2, String string) {
        return true;
    }

    public ab J() {
        return this.o;
    }

    public KeyPair K() {
        return this.G;
    }

    public int L() {
        return this.t;
    }

    public void b(int n2) {
        this.t = n2;
    }

    public String M() {
        return this.H;
    }

    public void j(String string) {
        this.H = string;
    }

    public boolean N() {
        return this.H != null;
    }

    public String O() {
        return this.I;
    }

    public void k(String string) {
        this.I = string;
    }

    public void a(KeyPair keyPair) {
        this.G = keyPair;
    }

    public void a(rd rd2) {
        for (int i2 = 0; i2 < this.c.length; ++i2) {
            mt mt2 = this.c[i2];
            if (mt2 == null) continue;
            if (mt2.N().t()) {
                mt2.r = rd.d;
                mt2.a(true, true);
                continue;
            }
            if (this.N()) {
                mt2.r = rd2;
                mt2.a(mt2.r != rd.a, true);
                continue;
            }
            mt2.r = rd2;
            mt2.a(this.Q(), this.z);
        }
    }

    protected boolean Q() {
        return true;
    }

    public boolean R() {
        return this.K;
    }

    public void b(boolean bl2) {
        this.K = bl2;
    }

    public void c(boolean bl2) {
        this.L = bl2;
    }

    public aze S() {
        return this.k;
    }

    public void U() {
        this.M = true;
        this.S().d();
        for (int i2 = 0; i2 < this.c.length; ++i2) {
            mt mt2 = this.c[i2];
            if (mt2 == null) continue;
            mt2.n();
        }
        this.S().e(this.c[0].M().g());
        this.r();
    }

    public String V() {
        return this.N;
    }

    public void m(String string) {
        this.N = string;
    }

    @Override
    public void a(ri ri2) {
        ri2.a("whitelist_enabled", false);
        ri2.a("whitelist_count", 0);
        ri2.a("players_current", this.C());
        ri2.a("players_max", this.D());
        ri2.a("players_seen", this.u.q().length);
        ri2.a("uses_auth", this.y);
        ri2.a("gui_state", this.ak() ? "enabled" : "disabled");
        ri2.a("run_time", (MinecraftServer.ar() - ri2.g()) / 60L * 1000L);
        ri2.a("avg_tick_ms", (int)(qh.a(this.g) * 1.0E-6));
        int n2 = 0;
        for (int i2 = 0; i2 < this.c.length; ++i2) {
            if (this.c[i2] == null) continue;
            mt mt2 = this.c[i2];
            ays ays2 = mt2.N();
            ri2.a("world[" + n2 + "][dimension]", mt2.t.i);
            ri2.a("world[" + n2 + "][mode]", (Object)ays2.r());
            ri2.a("world[" + n2 + "][difficulty]", (Object)mt2.r);
            ri2.a("world[" + n2 + "][hardcore]", ays2.t());
            ri2.a("world[" + n2 + "][generator_name]", ays2.u().a());
            ri2.a("world[" + n2 + "][generator_version]", ays2.u().d());
            ri2.a("world[" + n2 + "][height]", this.E);
            ri2.a("world[" + n2 + "][chunks_loaded]", mt2.L().g());
            ++n2;
        }
        ri2.a("worlds", n2);
    }

    @Override
    public void b(ri ri2) {
        ri2.b("singleplayer", this.N());
        ri2.b("server_brand", this.getServerModName());
        ri2.b("gui_supported", GraphicsEnvironment.isHeadless() ? "headless" : "supported");
        ri2.b("dedicated", this.X());
    }

    @Override
    public boolean W() {
        return true;
    }

    public abstract boolean X();

    public boolean Y() {
        return this.y;
    }

    public void d(boolean bl2) {
        this.y = bl2;
    }

    public boolean Z() {
        return this.z;
    }

    public void e(boolean bl2) {
        this.z = bl2;
    }

    public boolean aa() {
        return this.A;
    }

    public void f(boolean bl2) {
        this.A = bl2;
    }

    public boolean ab() {
        return this.B;
    }

    public void g(boolean bl2) {
        this.B = bl2;
    }

    public boolean ac() {
        return this.C;
    }

    public void h(boolean bl2) {
        this.C = bl2;
    }

    public abstract boolean ad();

    public String ae() {
        return this.D;
    }

    public void n(String string) {
        this.D = string;
    }

    public int af() {
        return this.E;
    }

    public void c(int n2) {
        this.E = n2;
    }

    public boolean ag() {
        return this.w;
    }

    public oi ah() {
        return this.u;
    }

    public void a(oi oi2) {
        this.u = oi2;
    }

    public void a(ahk ahk2) {
        for (int i2 = 0; i2 < this.c.length; ++i2) {
            MinecraftServer.I().c[i2].N().a(ahk2);
        }
    }

    public nc ai() {
        return this.p;
    }

    public boolean ak() {
        return false;
    }

    public abstract String a(ahk var1, boolean var2);

    public int al() {
        return this.x;
    }

    public void am() {
        this.R = true;
    }

    @Override
    public r f_() {
        return new r(0, 0, 0);
    }

    @Override
    public ahb d() {
        return this.c[0];
    }

    public int ao() {
        return 16;
    }

    public boolean a(ahb ahb2, int n2, int n3, int n4, yz yz2) {
        return false;
    }

    public void i(boolean bl2) {
        this.S = bl2;
    }

    public boolean ap() {
        return this.S;
    }

    public Proxy aq() {
        return this.d;
    }

    public static long ar() {
        return System.currentTimeMillis();
    }

    public int as() {
        return this.F;
    }

    public void d(int n2) {
        this.F = n2;
    }

    @Override
    public fj c_() {
        return new fq(this.b_());
    }

    public boolean at() {
        return true;
    }

    public MinecraftSessionService av() {
        return this.U;
    }

    public GameProfileRepository aw() {
        return this.W;
    }

    public ns ax() {
        return this.X;
    }

    public kb ay() {
        return this.q;
    }

    public void az() {
        this.V = 0L;
    }

    public static /* synthetic */ Logger aA() {
        return i;
    }

    public static /* synthetic */ oi a(MinecraftServer minecraftServer) {
        return minecraftServer.u;
    }
}

