/*
 * Decompiled with CFR 0.152.
 */
import com.google.common.base.Charsets;
import com.google.common.collect.Lists;
import io.netty.buffer.Unpooled;
import io.netty.util.concurrent.GenericFutureListener;
import java.io.ByteArrayInputStream;
import java.io.DataInputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Random;
import net.minecraft.server.MinecraftServer;
import org.apache.commons.lang3.StringUtils;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class nh
implements io {
    private static final Logger c = LogManager.getLogger();
    public final ej a;
    private final MinecraftServer d;
    public mw b;
    private int e;
    private int f;
    private boolean g;
    private int h;
    private long i;
    private static Random j = new Random();
    private long k;
    private int l;
    private int m;
    private pz n = new pz();
    private double o;
    private double p;
    private double q;
    private boolean r = true;

    public nh(MinecraftServer minecraftServer, ej ej2, mw mw2) {
        this.d = minecraftServer;
        this.a = ej2;
        ej2.a(this);
        this.b = mw2;
        mw2.a = this;
    }

    @Override
    public void a() {
        this.g = false;
        ++this.e;
        this.d.b.a("keepAlive");
        if ((long)this.e - this.k > 40L) {
            this.k = this.e;
            this.i = this.d();
            this.h = (int)this.i;
            this.a(new gw(this.h));
        }
        if (this.l > 0) {
            --this.l;
        }
        if (this.m > 0) {
            --this.m;
        }
        if (this.b.x() > 0L && this.d.as() > 0 && MinecraftServer.ar() - this.b.x() > (long)(this.d.as() * 1000 * 60)) {
            this.c("You have been idle for too long!");
        }
    }

    public ej b() {
        return this.a;
    }

    public void c(String string) {
        fq fq2 = new fq(string);
        this.a.a(new gs(fq2), new ni(this, fq2));
        this.a.g();
    }

    @Override
    public void a(jk jk2) {
        this.b.a(jk2.c(), jk2.d(), jk2.e(), jk2.f());
    }

    @Override
    public void a(jd jd2) {
        double d2;
        mt mt2 = this.d.a(this.b.ap);
        this.g = true;
        if (this.b.i) {
            return;
        }
        if (!this.r) {
            d2 = jd2.d() - this.p;
            if (jd2.c() == this.o && d2 * d2 < 0.01 && jd2.e() == this.q) {
                this.r = true;
            }
        }
        if (this.r) {
            double d3;
            double d4;
            double d5;
            if (this.b.m != null) {
                float f2 = this.b.y;
                float f3 = this.b.z;
                this.b.m.ac();
                double d6 = this.b.s;
                double d7 = this.b.t;
                double d8 = this.b.u;
                if (jd2.k()) {
                    f2 = jd2.g();
                    f3 = jd2.h();
                }
                this.b.D = jd2.i();
                this.b.i();
                this.b.V = 0.0f;
                this.b.a(d6, d7, d8, f2, f3);
                if (this.b.m != null) {
                    this.b.m.ac();
                }
                this.d.ah().d(this.b);
                if (this.r) {
                    this.o = this.b.s;
                    this.p = this.b.t;
                    this.q = this.b.u;
                }
                mt2.g(this.b);
                return;
            }
            if (this.b.bm()) {
                this.b.i();
                this.b.a(this.o, this.p, this.q, this.b.y, this.b.z);
                mt2.g(this.b);
                return;
            }
            d2 = this.b.t;
            this.o = this.b.s;
            this.p = this.b.t;
            this.q = this.b.u;
            double d9 = this.b.s;
            double d10 = this.b.t;
            double d11 = this.b.u;
            float f4 = this.b.y;
            float f5 = this.b.z;
            if (jd2.j() && jd2.d() == -999.0 && jd2.f() == -999.0) {
                jd2.a(false);
            }
            if (jd2.j()) {
                d9 = jd2.c();
                d10 = jd2.d();
                d11 = jd2.e();
                d5 = jd2.f() - jd2.d();
                if (!this.b.bm() && (d5 > 1.65 || d5 < 0.1)) {
                    this.c("Illegal stance");
                    c.warn(this.b.b_() + " had an illegal stance: " + d5);
                    return;
                }
                if (Math.abs(jd2.c()) > 3.2E7 || Math.abs(jd2.e()) > 3.2E7) {
                    this.c("Illegal position");
                    return;
                }
            }
            if (jd2.k()) {
                f4 = jd2.g();
                f5 = jd2.h();
            }
            this.b.i();
            this.b.V = 0.0f;
            this.b.a(this.o, this.p, this.q, f4, f5);
            if (!this.r) {
                return;
            }
            d5 = d9 - this.b.s;
            double d12 = d10 - this.b.t;
            double d13 = d11 - this.b.u;
            double d14 = Math.min(Math.abs(d5), Math.abs(this.b.v));
            double d15 = d14 * d14 + (d4 = Math.min(Math.abs(d12), Math.abs(this.b.w))) * d4 + (d3 = Math.min(Math.abs(d13), Math.abs(this.b.x))) * d3;
            if (!(!(d15 > 100.0) || this.d.N() && this.d.M().equals(this.b.b_()))) {
                c.warn(this.b.b_() + " moved too quickly! " + d5 + "," + d12 + "," + d13 + " (" + d14 + ", " + d4 + ", " + d3 + ")");
                this.a(this.o, this.p, this.q, this.b.y, this.b.z);
                return;
            }
            float f6 = 0.0625f;
            boolean bl2 = mt2.a((sa)this.b, this.b.C.b().e(f6, f6, f6)).isEmpty();
            if (this.b.D && !jd2.i() && d12 > 0.0) {
                this.b.bj();
            }
            this.b.d(d5, d12, d13);
            this.b.D = jd2.i();
            this.b.k(d5, d12, d13);
            double d16 = d12;
            d5 = d9 - this.b.s;
            d12 = d10 - this.b.t;
            if (d12 > -0.5 || d12 < 0.5) {
                d12 = 0.0;
            }
            d13 = d11 - this.b.u;
            d15 = d5 * d5 + d12 * d12 + d13 * d13;
            boolean bl3 = false;
            if (d15 > 0.0625 && !this.b.bm() && !this.b.c.d()) {
                bl3 = true;
                c.warn(this.b.b_() + " moved wrongly!");
            }
            this.b.a(d9, d10, d11, f4, f5);
            boolean bl4 = mt2.a((sa)this.b, this.b.C.b().e(f6, f6, f6)).isEmpty();
            if (bl2 && (bl3 || !bl4) && !this.b.bm()) {
                this.a(this.o, this.p, this.q, f4, f5);
                return;
            }
            azt azt2 = this.b.C.b().b(f6, f6, f6).a(0.0, -0.55, 0.0);
            if (!(this.d.ac() || this.b.c.d() || mt2.c(azt2))) {
                if (d16 >= -0.03125) {
                    ++this.f;
                    if (this.f > 80) {
                        c.warn(this.b.b_() + " was kicked for floating too long!");
                        this.c("Flying is not enabled on this server");
                        return;
                    }
                }
            } else {
                this.f = 0;
            }
            this.b.D = jd2.i();
            this.d.ah().d(this.b);
            this.b.b(this.b.t - d2, jd2.i());
        } else if (this.e % 20 == 0) {
            this.a(this.o, this.p, this.q, this.b.y, this.b.z);
        }
    }

    public void a(double d2, double d3, double d4, float f2, float f3) {
        this.r = false;
        this.o = d2;
        this.p = d3;
        this.q = d4;
        this.b.a(d2, d3, d4, f2, f3);
        this.b.a.a(new fu(d2, d3 + (double)1.62f, d4, f2, f3, false));
    }

    @Override
    public void a(ji ji2) {
        mt mt2 = this.d.a(this.b.ap);
        this.b.v();
        if (ji2.g() == 4) {
            this.b.a(false);
            return;
        }
        if (ji2.g() == 3) {
            this.b.a(true);
            return;
        }
        if (ji2.g() == 5) {
            this.b.bA();
            return;
        }
        boolean bl2 = false;
        if (ji2.g() == 0) {
            bl2 = true;
        }
        if (ji2.g() == 1) {
            bl2 = true;
        }
        if (ji2.g() == 2) {
            bl2 = true;
        }
        int n2 = ji2.c();
        int n3 = ji2.d();
        int n4 = ji2.e();
        if (bl2) {
            double d2 = this.b.s - ((double)n2 + 0.5);
            double d3 = this.b.t - ((double)n3 + 0.5) + 1.5;
            double d4 = this.b.u - ((double)n4 + 0.5);
            double d5 = d2 * d2 + d3 * d3 + d4 * d4;
            if (d5 > 36.0) {
                return;
            }
            if (n3 >= this.d.af()) {
                return;
            }
        }
        if (ji2.g() == 0) {
            if (!this.d.a(mt2, n2, n3, n4, this.b)) {
                this.b.c.a(n2, n3, n4, ji2.f());
            } else {
                this.b.a.a(new gh(n2, n3, n4, mt2));
            }
        } else if (ji2.g() == 2) {
            this.b.c.a(n2, n3, n4);
            if (mt2.a(n2, n3, n4).o() != awt.a) {
                this.b.a.a(new gh(n2, n3, n4, mt2));
            }
        } else if (ji2.g() == 1) {
            this.b.c.c(n2, n3, n4);
            if (mt2.a(n2, n3, n4).o() != awt.a) {
                this.b.a.a(new gh(n2, n3, n4, mt2));
            }
        }
    }

    @Override
    public void a(jo jo2) {
        Object object;
        mt mt2 = this.d.a(this.b.ap);
        add add2 = this.b.bm.h();
        boolean bl2 = false;
        int n2 = jo2.c();
        int n3 = jo2.d();
        int n4 = jo2.e();
        int n5 = jo2.f();
        this.b.v();
        if (jo2.f() == 255) {
            if (add2 == null) {
                return;
            }
            this.b.c.a(this.b, mt2, add2);
        } else if (jo2.d() < this.d.af() - 1 || jo2.f() != 1 && jo2.d() < this.d.af()) {
            if (this.r && this.b.e((double)n2 + 0.5, (double)n3 + 0.5, (double)n4 + 0.5) < 64.0 && !this.d.a(mt2, n2, n3, n4, this.b)) {
                this.b.c.a(this.b, mt2, add2, n2, n3, n4, n5, jo2.h(), jo2.i(), jo2.j());
            }
            bl2 = true;
        } else {
            object = new fr("build.tooHigh", this.d.af());
            ((fe)object).b().a(a.m);
            this.b.a.a(new gj((fj)object));
            bl2 = true;
        }
        if (bl2) {
            this.b.a.a(new gh(n2, n3, n4, mt2));
            if (n5 == 0) {
                --n3;
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
            this.b.a.a(new gh(n2, n3, n4, mt2));
        }
        if ((add2 = this.b.bm.h()) != null && add2.b == 0) {
            this.b.bm.a[this.b.bm.c] = null;
            add2 = null;
        }
        if (add2 == null || add2.n() == 0) {
            this.b.g = true;
            this.b.bm.a[this.b.bm.c] = add.b(this.b.bm.a[this.b.bm.c]);
            object = this.b.bo.a(this.b.bm, this.b.bm.c);
            this.b.bo.b();
            this.b.g = false;
            if (!add.b(this.b.bm.h(), jo2.g())) {
                this.a(new gq(this.b.bo.d, ((aay)object).g, this.b.bm.h()));
            }
        }
    }

    @Override
    public void a(fj fj2) {
        c.info(this.b.b_() + " lost connection: " + fj2);
        this.d.az();
        fr fr2 = new fr("multiplayer.player.left", this.b.c_());
        fr2.b().a(a.o);
        this.d.ah().a(fr2);
        this.b.n();
        this.d.ah().e(this.b);
        if (this.d.N() && this.b.b_().equals(this.d.M())) {
            c.info("Stopping singleplayer server as player logged out");
            this.d.r();
        }
    }

    public void a(ft ft2) {
        Object object;
        if (ft2 instanceof gj) {
            gj gj2 = (gj)ft2;
            object = this.b.u();
            if (object == zb.c) {
                return;
            }
            if (object == zb.b && !gj2.d()) {
                return;
            }
        }
        try {
            this.a.a(ft2, new GenericFutureListener[0]);
        }
        catch (Throwable throwable) {
            object = b.a(throwable, "Sending packet");
            k k2 = ((b)object).a("Packet being sent");
            k2.a("Packet class", new nj(this, ft2));
            throw new s((b)object);
        }
    }

    @Override
    public void a(jl jl2) {
        if (jl2.c() < 0 || jl2.c() >= yx.i()) {
            c.warn(this.b.b_() + " tried to set an invalid carried item");
            return;
        }
        this.b.bm.c = jl2.c();
        this.b.v();
    }

    @Override
    public void a(ir ir2) {
        if (this.b.u() == zb.c) {
            fr fr2 = new fr("chat.cannotSend", new Object[0]);
            fr2.b().a(a.m);
            this.a(new gj(fr2));
            return;
        }
        this.b.v();
        String string = ir2.c();
        string = StringUtils.normalizeSpace(string);
        for (int i2 = 0; i2 < string.length(); ++i2) {
            if (t.a(string.charAt(i2))) continue;
            this.c("Illegal characters in chat");
            return;
        }
        if (string.startsWith("/")) {
            this.d(string);
        } else {
            fr fr3 = new fr("chat.type.text", this.b.c_(), string);
            this.d.ah().a(fr3, false);
        }
        this.l += 20;
        if (this.l > 200 && !this.d.ah().g(this.b.bJ())) {
            this.c("disconnect.spam");
        }
    }

    private void d(String string) {
        this.d.J().a(this.b, string);
    }

    @Override
    public void a(ip ip2) {
        this.b.v();
        if (ip2.d() == 1) {
            this.b.ba();
        }
    }

    @Override
    public void a(jj jj2) {
        this.b.v();
        if (jj2.d() == 1) {
            this.b.b(true);
        } else if (jj2.d() == 2) {
            this.b.b(false);
        } else if (jj2.d() == 4) {
            this.b.c(true);
        } else if (jj2.d() == 5) {
            this.b.c(false);
        } else if (jj2.d() == 3) {
            this.b.a(false, true, true);
            this.r = false;
        } else if (jj2.d() == 6) {
            if (this.b.m != null && this.b.m instanceof wi) {
                ((wi)this.b.m).w(jj2.e());
            }
        } else if (jj2.d() == 7 && this.b.m != null && this.b.m instanceof wi) {
            ((wi)this.b.m).g(this.b);
        }
    }

    @Override
    public void a(ja ja2) {
        mt mt2 = this.d.a(this.b.ap);
        sa sa2 = ja2.a(mt2);
        this.b.v();
        if (sa2 != null) {
            boolean bl2 = this.b.p(sa2);
            double d2 = 36.0;
            if (!bl2) {
                d2 = 9.0;
            }
            if (this.b.f(sa2) < d2) {
                if (ja2.c() == jb.a) {
                    this.b.q(sa2);
                } else if (ja2.c() == jb.b) {
                    if (sa2 instanceof xk || sa2 instanceof sq || sa2 instanceof zc || sa2 == this.b) {
                        this.c("Attempting to attack an invalid entity");
                        this.d.f("Player " + this.b.b_() + " tried to attack an invalid entity");
                        return;
                    }
                    this.b.r(sa2);
                }
            }
        }
    }

    @Override
    public void a(is is2) {
        this.b.v();
        it it2 = is2.c();
        switch (it2) {
            case a: {
                if (this.b.i) {
                    this.b = this.d.ah().a(this.b, 0, true);
                    break;
                }
                if (this.b.r().N().t()) {
                    if (this.d.N() && this.b.b_().equals(this.d.M())) {
                        this.b.a.c("You have died. Game over, man, it's game over!");
                        this.d.U();
                        break;
                    }
                    oq oq2 = new oq(this.b.bJ(), null, "(You just lost the game)", null, "Death in Hardcore");
                    this.d.ah().h().a(oq2);
                    this.b.a.c("You have died. Game over, man, it's game over!");
                    break;
                }
                if (this.b.aS() > 0.0f) {
                    return;
                }
                this.b = this.d.ah().a(this.b, 0, false);
                break;
            }
            case b: {
                this.b.w().a(this.b);
                break;
            }
            case c: {
                this.b.a(pc.f);
            }
        }
    }

    @Override
    public void a(iy iy2) {
        this.b.m();
    }

    @Override
    public void a(ix ix2) {
        this.b.v();
        if (this.b.bo.d == ix2.c() && this.b.bo.c(this.b)) {
            add add2 = this.b.bo.a(ix2.d(), ix2.e(), ix2.h(), (yz)this.b);
            if (add.b(ix2.g(), add2)) {
                this.b.a.a(new gl(ix2.c(), ix2.f(), true));
                this.b.g = true;
                this.b.bo.b();
                this.b.l();
                this.b.g = false;
            } else {
                this.n.a(this.b.bo.d, ix2.f());
                this.b.a.a(new gl(ix2.c(), ix2.f(), false));
                this.b.bo.a((yz)this.b, false);
                ArrayList<add> arrayList = new ArrayList<add>();
                for (int i2 = 0; i2 < this.b.bo.c.size(); ++i2) {
                    arrayList.add(((aay)this.b.bo.c.get(i2)).d());
                }
                this.b.a(this.b.bo, arrayList);
            }
        }
    }

    @Override
    public void a(iw iw2) {
        this.b.v();
        if (this.b.bo.d == iw2.c() && this.b.bo.c(this.b)) {
            this.b.bo.a((yz)this.b, iw2.d());
            this.b.bo.b();
        }
    }

    @Override
    public void a(jm jm2) {
        if (this.b.c.d()) {
            boolean bl2;
            boolean bl3 = jm2.c() < 0;
            add add2 = jm2.d();
            boolean bl4 = jm2.c() >= 1 && jm2.c() < 36 + yx.i();
            boolean bl5 = add2 == null || add2.b() != null;
            boolean bl6 = bl2 = add2 == null || add2.k() >= 0 && add2.b <= 64 && add2.b > 0;
            if (bl4 && bl5 && bl2) {
                if (add2 == null) {
                    this.b.bn.a(jm2.c(), null);
                } else {
                    this.b.bn.a(jm2.c(), add2);
                }
                this.b.bn.a((yz)this.b, true);
            } else if (bl3 && bl5 && bl2 && this.m < 200) {
                this.m += 20;
                xk xk2 = this.b.a(add2, true);
                if (xk2 != null) {
                    xk2.e();
                }
            }
        }
    }

    @Override
    public void a(iv iv2) {
        Short s2 = (Short)this.n.a(this.b.bo.d);
        if (s2 != null && iv2.d() == s2.shortValue() && this.b.bo.d == iv2.c() && !this.b.bo.c(this.b)) {
            this.b.bo.a((yz)this.b, true);
        }
    }

    @Override
    public void a(jn jn2) {
        this.b.v();
        mt mt2 = this.d.a(this.b.ap);
        if (mt2.d(jn2.c(), jn2.d(), jn2.e())) {
            int n2;
            int n3;
            int n4;
            apm apm2;
            aor aor2 = mt2.o(jn2.c(), jn2.d(), jn2.e());
            if (aor2 instanceof apm && (!(apm2 = (apm)aor2).a() || apm2.b() != this.b)) {
                this.d.f("Player " + this.b.b_() + " just tried to change non-editable sign");
                return;
            }
            for (n4 = 0; n4 < 4; ++n4) {
                n3 = 1;
                if (jn2.f()[n4].length() > 15) {
                    n3 = 0;
                } else {
                    for (n2 = 0; n2 < jn2.f()[n4].length(); ++n2) {
                        if (t.a(jn2.f()[n4].charAt(n2))) continue;
                        n3 = 0;
                    }
                }
                if (n3 != 0) continue;
                jn2.f()[n4] = "!?";
            }
            if (aor2 instanceof apm) {
                n4 = jn2.c();
                n3 = jn2.d();
                n2 = jn2.e();
                apm apm3 = (apm)aor2;
                System.arraycopy(jn2.f(), 0, apm3.a, 0, 4);
                apm3.e();
                mt2.g(n4, n3, n2);
            }
        }
    }

    @Override
    public void a(jc jc2) {
        if (jc2.c() == this.h) {
            int n2 = (int)(this.d() - this.i);
            this.b.h = (this.b.h * 3 + n2) / 4;
        }
    }

    private long d() {
        return System.nanoTime() / 1000000L;
    }

    @Override
    public void a(jh jh2) {
        this.b.bE.b = jh2.d() && this.b.bE.c;
    }

    @Override
    public void a(iq iq2) {
        ArrayList<String> arrayList = Lists.newArrayList();
        for (String string : this.d.a(this.b, iq2.c())) {
            arrayList.add(string);
        }
        this.b.a.a(new gi(arrayList.toArray(new String[arrayList.size()])));
    }

    @Override
    public void a(iu iu2) {
        this.b.a(iu2);
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     * Enabled force condition propagation
     * Lifted jumps to return sites
     */
    @Override
    public void a(iz iz2) {
        if ("MC|BEdit".equals(iz2.c())) {
            et et2 = new et(Unpooled.wrappedBuffer(iz2.e()));
            try {
                add add2 = et2.c();
                if (add2 == null) {
                    return;
                }
                if (!ael.a(add2.q())) {
                    throw new IOException("Invalid book tag!");
                }
                add add3 = this.b.bm.h();
                if (add3 == null) {
                    return;
                }
                if (add2.b() != ade.bA || add2.b() != add3.b()) return;
                add3.a("pages", add2.q().c("pages", 8));
                return;
            }
            catch (Exception exception) {
                c.error("Couldn't handle book info", (Throwable)exception);
                return;
            }
            finally {
                et2.release();
            }
        } else if ("MC|BSign".equals(iz2.c())) {
            et et3 = new et(Unpooled.wrappedBuffer(iz2.e()));
            try {
                add add4 = et3.c();
                if (add4 == null) {
                    return;
                }
                if (!aem.a(add4.q())) {
                    throw new IOException("Invalid book tag!");
                }
                add add5 = this.b.bm.h();
                if (add5 == null) {
                    return;
                }
                if (add4.b() != ade.bB || add5.b() != ade.bA) return;
                add5.a("author", new dx(this.b.b_()));
                add5.a("title", new dx(add4.q().j("title")));
                add5.a("pages", add4.q().c("pages", 8));
                add5.a(ade.bB);
                return;
            }
            catch (Exception exception) {
                c.error("Couldn't sign book", (Throwable)exception);
                return;
            }
            finally {
                et3.release();
            }
        } else if ("MC|TrSel".equals(iz2.c())) {
            try {
                DataInputStream dataInputStream = new DataInputStream(new ByteArrayInputStream(iz2.e()));
                int n2 = dataInputStream.readInt();
                zs zs2 = this.b.bo;
                if (!(zs2 instanceof aat)) return;
                ((aat)zs2).e(n2);
                return;
            }
            catch (Exception exception) {
                c.error("Couldn't select trade", (Throwable)exception);
            }
            return;
        } else if ("MC|AdvCdm".equals(iz2.c())) {
            if (!this.d.ad()) {
                this.b.a(new fr("advMode.notEnabled", new Object[0]));
                return;
            } else if (this.b.a(2, "") && this.b.bE.d) {
                et et4 = new et(Unpooled.wrappedBuffer(iz2.e()));
                try {
                    Object object;
                    byte by2 = et4.readByte();
                    agp agp2 = null;
                    if (by2 == 0) {
                        object = this.b.o.o(et4.readInt(), et4.readInt(), et4.readInt());
                        if (object instanceof aox) {
                            agp2 = ((aox)object).a();
                        }
                    } else if (by2 == 1 && (object = this.b.o.a(et4.readInt())) instanceof xn) {
                        agp2 = ((xn)object).e();
                    }
                    object = et4.c(et4.readableBytes());
                    if (agp2 == null) return;
                    agp2.a((String)object);
                    agp2.e();
                    this.b.a(new fr("advMode.setCommand.success", object));
                    return;
                }
                catch (Exception exception) {
                    c.error("Couldn't set command block", (Throwable)exception);
                    return;
                }
                finally {
                    et4.release();
                }
            } else {
                this.b.a(new fr("advMode.notAllowed", new Object[0]));
            }
            return;
        } else if ("MC|Beacon".equals(iz2.c())) {
            if (!(this.b.bo instanceof zx)) return;
            try {
                DataInputStream dataInputStream = new DataInputStream(new ByteArrayInputStream(iz2.e()));
                int n3 = dataInputStream.readInt();
                int n4 = dataInputStream.readInt();
                zx zx2 = (zx)this.b.bo;
                aay aay2 = zx2.a(0);
                if (!aay2.e()) return;
                aay2.a(1);
                aoq aoq2 = zx2.e();
                aoq2.d(n3);
                aoq2.e(n4);
                aoq2.e();
                return;
            }
            catch (Exception exception) {
                c.error("Couldn't set beacon", (Throwable)exception);
            }
            return;
        } else {
            if (!"MC|ItemName".equals(iz2.c()) || !(this.b.bo instanceof zu)) return;
            zu zu2 = (zu)this.b.bo;
            if (iz2.e() == null || iz2.e().length < 1) {
                zu2.a("");
                return;
            } else {
                String string = t.a(new String(iz2.e(), Charsets.UTF_8));
                if (string.length() > 30) return;
                zu2.a(string);
            }
        }
    }

    @Override
    public void a(eo eo2, eo eo3) {
        if (eo3 != eo.b) {
            throw new IllegalStateException("Unexpected change in protocol!");
        }
    }
}

