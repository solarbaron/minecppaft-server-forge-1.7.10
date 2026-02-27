/*
 * Decompiled with CFR 0.152.
 */
import java.io.File;
import java.io.IOException;
import java.net.InetAddress;
import java.net.Proxy;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Random;
import net.minecraft.server.MinecraftServer;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class lt
extends MinecraftServer
implements lo {
    private static final Logger i = LogManager.getLogger();
    private final List j = Collections.synchronizedList(new ArrayList());
    private ox k;
    private pa l;
    private lq m;
    private lg n;
    private boolean o;
    private ahk p;
    private boolean q;

    public lt(File file) {
        super(file, Proxy.NO_PROXY);
        new lu(this, "Server Infinisleeper");
    }

    @Override
    protected boolean e() {
        ahm ahm2;
        lv lv2 = new lv(this, "Server console handler");
        lv2.setDaemon(true);
        lv2.start();
        i.info("Starting minecraft server version 1.7.10");
        if (Runtime.getRuntime().maxMemory() / 1024L / 1024L < 512L) {
            i.warn("To start the server with more ram, launch it as \"java -Xmx1024M -Xms1024M -jar minecraft_server.jar\"");
        }
        i.info("Loading properties");
        this.m = new lq(new File("server.properties"));
        this.n = new lg(new File("eula.txt"));
        if (!this.n.a()) {
            i.info("You need to agree to the EULA in order to run the server. Go to eula.txt for more info.");
            this.n.b();
            return false;
        }
        if (this.N()) {
            this.c("127.0.0.1");
        } else {
            this.d(this.m.a("online-mode", true));
            this.c(this.m.a("server-ip", ""));
        }
        this.e(this.m.a("spawn-animals", true));
        this.f(this.m.a("spawn-npcs", true));
        this.g(this.m.a("pvp", true));
        this.h(this.m.a("allow-flight", false));
        this.m(this.m.a("resource-pack", ""));
        this.n(this.m.a("motd", "A Minecraft Server"));
        this.i(this.m.a("force-gamemode", false));
        this.d(this.m.a("player-idle-timeout", 0));
        if (this.m.a("difficulty", 1) < 0) {
            this.m.a("difficulty", (Object)0);
        } else if (this.m.a("difficulty", 1) > 3) {
            this.m.a("difficulty", (Object)3);
        }
        this.o = this.m.a("generate-structures", true);
        int n2 = this.m.a("gamemode", ahk.b.a());
        this.p = ahj.a(n2);
        i.info("Default game type: " + (Object)((Object)this.p));
        InetAddress inetAddress = null;
        if (this.p().length() > 0) {
            inetAddress = InetAddress.getByName(this.p());
        }
        if (this.L() < 0) {
            this.b(this.m.a("server-port", 25565));
        }
        i.info("Generating keypair");
        this.a(pt.b());
        i.info("Starting Minecraft server on " + (this.p().length() == 0 ? "*" : this.p()) + ":" + this.L());
        try {
            this.ai().a(inetAddress, this.L());
        }
        catch (IOException iOException) {
            i.warn("**** FAILED TO BIND TO PORT!");
            i.warn("The exception was: {}", iOException.toString());
            i.warn("Perhaps a server is already running on that port?");
            return false;
        }
        if (!this.Y()) {
            i.warn("**** SERVER IS RUNNING IN OFFLINE/INSECURE MODE!");
            i.warn("The server will make no attempt to authenticate usernames. Beware.");
            i.warn("While this makes the game possible to play without internet access, it also opens up the ability for hackers to connect with any username they choose.");
            i.warn("To change this, set \"online-mode\" to \"true\" in the server.properties file.");
        }
        if (this.aE()) {
            this.ax().c();
        }
        if (!nz.a(this.m)) {
            return false;
        }
        this.a(new ls(this));
        long l2 = System.nanoTime();
        if (this.O() == null) {
            this.k(this.m.a("level-name", "world"));
        }
        String string = this.m.a("level-seed", "");
        String string2 = this.m.a("level-type", "DEFAULT");
        String string3 = this.m.a("generator-settings", "");
        long l3 = new Random().nextLong();
        if (string.length() > 0) {
            try {
                long l4 = Long.parseLong(string);
                if (l4 != 0L) {
                    l3 = l4;
                }
            }
            catch (NumberFormatException numberFormatException) {
                l3 = string.hashCode();
            }
        }
        if ((ahm2 = ahm.a(string2)) == null) {
            ahm2 = ahm.b;
        }
        this.at();
        this.ad();
        this.l();
        this.W();
        this.c(this.m.a("max-build-height", 256));
        this.c((this.af() + 8) / 16 * 16);
        this.c(qh.a(this.af(), 64, 256));
        this.m.a("max-build-height", (Object)this.af());
        i.info("Preparing level \"" + this.O() + "\"");
        this.a(this.O(), this.O(), l3, ahm2, string3);
        long l5 = System.nanoTime() - l2;
        String string4 = String.format("%.3fs", (double)l5 / 1.0E9);
        i.info("Done (" + string4 + ")! For help, type \"help\" or \"?\"");
        if (this.m.a("enable-query", false)) {
            i.info("Starting GS4 status listener");
            this.k = new ox(this);
            this.k.a();
        }
        if (this.m.a("enable-rcon", false)) {
            i.info("Starting remote control listener");
            this.l = new pa(this);
            this.l.a();
        }
        return true;
    }

    @Override
    public boolean h() {
        return this.o;
    }

    @Override
    public ahk i() {
        return this.p;
    }

    @Override
    public rd j() {
        return rd.a(this.m.a("difficulty", 1));
    }

    @Override
    public boolean k() {
        return this.m.a("hardcore", false);
    }

    @Override
    protected void a(b b2) {
    }

    @Override
    public b b(b b2) {
        b2 = super.b(b2);
        b2.g().a("Is Modded", new lw(this));
        b2.g().a("Type", new lx(this));
        return b2;
    }

    @Override
    protected void t() {
        System.exit(0);
    }

    @Override
    protected void v() {
        super.v();
        this.aB();
    }

    @Override
    public boolean w() {
        return this.m.a("allow-nether", true);
    }

    @Override
    public boolean Q() {
        return this.m.a("spawn-monsters", true);
    }

    @Override
    public void a(ri ri2) {
        ri2.a("whitelist_enabled", this.aC().r());
        ri2.a("whitelist_count", this.aC().l().length);
        super.a(ri2);
    }

    @Override
    public boolean W() {
        return this.m.a("snooper-enabled", true);
    }

    public void a(String string, ac ac2) {
        this.j.add(new le(string, ac2));
    }

    public void aB() {
        while (!this.j.isEmpty()) {
            le le2 = (le)this.j.remove(0);
            this.J().a(le2.b, le2.a);
        }
    }

    @Override
    public boolean X() {
        return true;
    }

    public ls aC() {
        return (ls)super.ah();
    }

    @Override
    public int a(String string, int n2) {
        return this.m.a(string, n2);
    }

    @Override
    public String a(String string, String string2) {
        return this.m.a(string, string2);
    }

    public boolean a(String string, boolean bl2) {
        return this.m.a(string, bl2);
    }

    @Override
    public void a(String string, Object object) {
        this.m.a(string, object);
    }

    @Override
    public void a() {
        this.m.b();
    }

    @Override
    public String b() {
        File file = this.m.c();
        if (file != null) {
            return file.getAbsolutePath();
        }
        return "No settings file";
    }

    public void aD() {
        ly.a(this);
        this.q = true;
    }

    @Override
    public boolean ak() {
        return this.q;
    }

    @Override
    public String a(ahk ahk2, boolean bl2) {
        return "";
    }

    @Override
    public boolean ad() {
        return this.m.a("enable-command-block", false);
    }

    @Override
    public int ao() {
        return this.m.a("spawn-protection", super.ao());
    }

    @Override
    public boolean a(ahb ahb2, int n2, int n3, int n4, yz yz2) {
        int n5;
        if (ahb2.t.i != 0) {
            return false;
        }
        if (this.aC().m().d()) {
            return false;
        }
        if (this.aC().g(yz2.bJ())) {
            return false;
        }
        if (this.ao() <= 0) {
            return false;
        }
        r r2 = ahb2.K();
        int n6 = qh.a(n2 - r2.a);
        int n7 = Math.max(n6, n5 = qh.a(n4 - r2.c));
        return n7 <= this.ao();
    }

    @Override
    public int l() {
        return this.m.a("op-permission-level", 4);
    }

    @Override
    public void d(int n2) {
        super.d(n2);
        this.m.a("player-idle-timeout", (Object)n2);
        this.a();
    }

    @Override
    public boolean m() {
        return this.m.a("broadcast-rcon-to-ops", true);
    }

    @Override
    public boolean at() {
        return this.m.a("announce-player-achievements", true);
    }

    protected boolean aE() {
        int n2;
        boolean bl2 = false;
        for (n2 = 0; !bl2 && n2 <= 2; ++n2) {
            if (n2 > 0) {
                i.warn("Encountered a problem while converting the user banlist, retrying in a few seconds");
                this.aG();
            }
            bl2 = nz.a(this);
        }
        boolean bl3 = false;
        for (n2 = 0; !bl3 && n2 <= 2; ++n2) {
            if (n2 > 0) {
                i.warn("Encountered a problem while converting the ip banlist, retrying in a few seconds");
                this.aG();
            }
            bl3 = nz.b(this);
        }
        boolean bl4 = false;
        for (n2 = 0; !bl4 && n2 <= 2; ++n2) {
            if (n2 > 0) {
                i.warn("Encountered a problem while converting the op list, retrying in a few seconds");
                this.aG();
            }
            bl4 = nz.c(this);
        }
        boolean bl5 = false;
        for (n2 = 0; !bl5 && n2 <= 2; ++n2) {
            if (n2 > 0) {
                i.warn("Encountered a problem while converting the whitelist, retrying in a few seconds");
                this.aG();
            }
            bl5 = nz.d(this);
        }
        boolean bl6 = false;
        for (n2 = 0; !bl6 && n2 <= 2; ++n2) {
            if (n2 > 0) {
                i.warn("Encountered a problem while converting the player save files, retrying in a few seconds");
                this.aG();
            }
            bl6 = nz.a(this, this.m);
        }
        return bl2 || bl3 || bl4 || bl5 || bl6;
    }

    private void aG() {
        try {
            Thread.sleep(5000L);
        }
        catch (InterruptedException interruptedException) {
            return;
        }
    }

    @Override
    public /* synthetic */ oi ah() {
        return this.aC();
    }

    static /* synthetic */ Logger aF() {
        return i;
    }
}

