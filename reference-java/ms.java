/*
 * Decompiled with CFR 0.152.
 */
import com.google.common.collect.Lists;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class ms
implements apu {
    private static final Logger b = LogManager.getLogger();
    private Set c = Collections.newSetFromMap(new ConcurrentHashMap());
    private apx d;
    private apu e;
    private aqc f;
    public boolean a = true;
    private qd g = new qd();
    private List h = new ArrayList();
    private mt i;

    public ms(mt mt2, aqc aqc2, apu apu2) {
        this.d = new apw(mt2, 0, 0);
        this.i = mt2;
        this.f = aqc2;
        this.e = apu2;
    }

    @Override
    public boolean a(int n2, int n3) {
        return this.g.b(agu.a(n2, n3));
    }

    public List a() {
        return this.h;
    }

    public void b(int n2, int n3) {
        if (this.i.t.e()) {
            r r2 = this.i.K();
            int n4 = n2 * 16 + 8 - r2.a;
            int n5 = n3 * 16 + 8 - r2.c;
            int n6 = 128;
            if (n4 < -n6 || n4 > n6 || n5 < -n6 || n5 > n6) {
                this.c.add(agu.a(n2, n3));
            }
        } else {
            this.c.add(agu.a(n2, n3));
        }
    }

    public void b() {
        for (apx apx2 : this.h) {
            this.b(apx2.g, apx2.h);
        }
    }

    @Override
    public apx c(int n2, int n3) {
        long l2 = agu.a(n2, n3);
        this.c.remove(l2);
        apx apx2 = (apx)this.g.a(l2);
        if (apx2 == null) {
            apx2 = this.f(n2, n3);
            if (apx2 == null) {
                if (this.e == null) {
                    apx2 = this.d;
                } else {
                    try {
                        apx2 = this.e.d(n2, n3);
                    }
                    catch (Throwable throwable) {
                        b b2 = b.a(throwable, "Exception generating new chunk");
                        k k2 = b2.a("Chunk to be generated");
                        k2.a("Location", String.format("%d,%d", n2, n3));
                        k2.a("Position hash", l2);
                        k2.a("Generator", this.e.f());
                        throw new s(b2);
                    }
                }
            }
            this.g.a(l2, apx2);
            this.h.add(apx2);
            apx2.c();
            apx2.a(this, this, n2, n3);
        }
        return apx2;
    }

    @Override
    public apx d(int n2, int n3) {
        apx apx2 = (apx)this.g.a(agu.a(n2, n3));
        if (apx2 == null) {
            if (this.i.y || this.a) {
                return this.c(n2, n3);
            }
            return this.d;
        }
        return apx2;
    }

    private apx f(int n2, int n3) {
        if (this.f == null) {
            return null;
        }
        try {
            apx apx2 = this.f.a(this.i, n2, n3);
            if (apx2 != null) {
                apx2.p = this.i.I();
                if (this.e != null) {
                    this.e.e(n2, n3);
                }
            }
            return apx2;
        }
        catch (Exception exception) {
            b.error("Couldn't load chunk", (Throwable)exception);
            return null;
        }
    }

    private void a(apx apx2) {
        if (this.f == null) {
            return;
        }
        try {
            this.f.b(this.i, apx2);
        }
        catch (Exception exception) {
            b.error("Couldn't save entities", (Throwable)exception);
        }
    }

    private void b(apx apx2) {
        if (this.f == null) {
            return;
        }
        try {
            apx2.p = this.i.I();
            this.f.a(this.i, apx2);
        }
        catch (IOException iOException) {
            b.error("Couldn't save chunk", (Throwable)iOException);
        }
        catch (ahg ahg2) {
            b.error("Couldn't save chunk; already in use by another instance of Minecraft?", (Throwable)ahg2);
        }
    }

    @Override
    public void a(apu apu2, int n2, int n3) {
        apx apx2 = this.d(n2, n3);
        if (!apx2.k) {
            apx2.p();
            if (this.e != null) {
                this.e.a(apu2, n2, n3);
                apx2.e();
            }
        }
    }

    @Override
    public boolean a(boolean bl2, qk qk2) {
        int n2 = 0;
        ArrayList arrayList = Lists.newArrayList(this.h);
        for (int i2 = 0; i2 < arrayList.size(); ++i2) {
            apx apx2 = (apx)arrayList.get(i2);
            if (bl2) {
                this.a(apx2);
            }
            if (!apx2.a(bl2)) continue;
            this.b(apx2);
            apx2.n = false;
            if (++n2 != 24 || bl2) continue;
            return false;
        }
        return true;
    }

    @Override
    public void c() {
        if (this.f != null) {
            this.f.b();
        }
    }

    @Override
    public boolean d() {
        if (!this.i.c) {
            for (int i2 = 0; i2 < 100; ++i2) {
                if (this.c.isEmpty()) continue;
                Long l2 = (Long)this.c.iterator().next();
                apx apx2 = (apx)this.g.a(l2);
                if (apx2 != null) {
                    apx2.d();
                    this.b(apx2);
                    this.a(apx2);
                    this.h.remove(apx2);
                }
                this.c.remove(l2);
                this.g.d(l2);
            }
            if (this.f != null) {
                this.f.a();
            }
        }
        return this.e.d();
    }

    @Override
    public boolean e() {
        return !this.i.c;
    }

    @Override
    public String f() {
        return "ServerChunkCache: " + this.g.a() + " Drop: " + this.c.size();
    }

    @Override
    public List a(sx sx2, int n2, int n3, int n4) {
        return this.e.a(sx2, n2, n3, n4);
    }

    @Override
    public agt a(ahb ahb2, String string, int n2, int n3, int n4) {
        return this.e.a(ahb2, string, n2, n3, n4);
    }

    @Override
    public int g() {
        return this.g.a();
    }

    @Override
    public void e(int n2, int n3) {
    }
}

