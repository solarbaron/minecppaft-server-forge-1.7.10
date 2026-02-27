/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Random;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class apx {
    private static final Logger t = LogManager.getLogger();
    public static boolean a;
    private apz[] u = new apz[16];
    private byte[] v = new byte[256];
    public int[] b = new int[256];
    public boolean[] c = new boolean[256];
    public boolean d;
    public ahb e;
    public int[] f;
    public final int g;
    public final int h;
    private boolean w;
    public Map i = new HashMap();
    public List[] j = new List[16];
    public boolean k;
    public boolean l;
    public boolean m;
    public boolean n;
    public boolean o;
    public long p;
    public boolean q;
    public int r;
    public long s;
    private int x = 4096;

    public apx(ahb ahb2, int n2, int n3) {
        this.e = ahb2;
        this.g = n2;
        this.h = n3;
        this.f = new int[256];
        for (int i2 = 0; i2 < this.j.length; ++i2) {
            this.j[i2] = new ArrayList();
        }
        Arrays.fill(this.b, -999);
        Arrays.fill(this.v, (byte)-1);
    }

    public apx(ahb ahb2, aji[] ajiArray, int n2, int n3) {
        this(ahb2, n2, n3);
        int n4 = ajiArray.length / 256;
        boolean bl2 = !ahb2.t.g;
        for (int i2 = 0; i2 < 16; ++i2) {
            for (int i3 = 0; i3 < 16; ++i3) {
                for (int i4 = 0; i4 < n4; ++i4) {
                    aji aji2 = ajiArray[i2 << 11 | i3 << 7 | i4];
                    if (aji2 == null || aji2.o() == awt.a) continue;
                    int n5 = i4 >> 4;
                    if (this.u[n5] == null) {
                        this.u[n5] = new apz(n5 << 4, bl2);
                    }
                    this.u[n5].a(i2, i4 & 0xF, i3, aji2);
                }
            }
        }
    }

    public apx(ahb ahb2, aji[] ajiArray, byte[] byArray, int n2, int n3) {
        this(ahb2, n2, n3);
        int n4 = ajiArray.length / 256;
        boolean bl2 = !ahb2.t.g;
        for (int i2 = 0; i2 < 16; ++i2) {
            for (int i3 = 0; i3 < 16; ++i3) {
                for (int i4 = 0; i4 < n4; ++i4) {
                    int n5 = i2 * n4 * 16 | i3 * n4 | i4;
                    aji aji2 = ajiArray[n5];
                    if (aji2 == null || aji2 == ajn.a) continue;
                    int n6 = i4 >> 4;
                    if (this.u[n6] == null) {
                        this.u[n6] = new apz(n6 << 4, bl2);
                    }
                    this.u[n6].a(i2, i4 & 0xF, i3, aji2);
                    this.u[n6].a(i2, i4 & 0xF, i3, byArray[n5]);
                }
            }
        }
    }

    public boolean a(int n2, int n3) {
        return n2 == this.g && n3 == this.h;
    }

    public int b(int n2, int n3) {
        return this.f[n3 << 4 | n2];
    }

    public int h() {
        for (int i2 = this.u.length - 1; i2 >= 0; --i2) {
            if (this.u[i2] == null) continue;
            return this.u[i2].d();
        }
        return 0;
    }

    public apz[] i() {
        return this.u;
    }

    public void b() {
        int n2 = this.h();
        this.r = Integer.MAX_VALUE;
        for (int i2 = 0; i2 < 16; ++i2) {
            for (int i3 = 0; i3 < 16; ++i3) {
                int n3;
                this.b[i2 + (i3 << 4)] = -999;
                for (n3 = n2 + 16 - 1; n3 > 0; --n3) {
                    if (this.b(i2, n3 - 1, i3) == 0) continue;
                    this.f[i3 << 4 | i2] = n3;
                    if (n3 >= this.r) break;
                    this.r = n3;
                    break;
                }
                if (this.e.t.g) continue;
                n3 = 15;
                int n4 = n2 + 16 - 1;
                do {
                    apz apz2;
                    int n5;
                    if ((n5 = this.b(i2, n4, i3)) == 0 && n3 != 15) {
                        n5 = 1;
                    }
                    if ((n3 -= n5) <= 0 || (apz2 = this.u[n4 >> 4]) == null) continue;
                    apz2.b(i2, n4 & 0xF, i3, n3);
                    this.e.m((this.g << 4) + i2, n4, (this.h << 4) + i3);
                } while (--n4 > 0 && n3 > 0);
            }
        }
        this.n = true;
    }

    private void e(int n2, int n3) {
        this.c[n2 + n3 * 16] = true;
        this.w = true;
    }

    private void c(boolean bl2) {
        this.e.C.a("recheckGaps");
        if (this.e.a(this.g * 16 + 8, 0, this.h * 16 + 8, 16)) {
            for (int i2 = 0; i2 < 16; ++i2) {
                for (int i3 = 0; i3 < 16; ++i3) {
                    if (!this.c[i2 + i3 * 16]) continue;
                    this.c[i2 + i3 * 16] = false;
                    int n2 = this.b(i2, i3);
                    int n3 = this.g * 16 + i2;
                    int n4 = this.h * 16 + i3;
                    int n5 = this.e.g(n3 - 1, n4);
                    int n6 = this.e.g(n3 + 1, n4);
                    int n7 = this.e.g(n3, n4 - 1);
                    int n8 = this.e.g(n3, n4 + 1);
                    if (n6 < n5) {
                        n5 = n6;
                    }
                    if (n7 < n5) {
                        n5 = n7;
                    }
                    if (n8 < n5) {
                        n5 = n8;
                    }
                    this.g(n3, n4, n5);
                    this.g(n3 - 1, n4, n2);
                    this.g(n3 + 1, n4, n2);
                    this.g(n3, n4 - 1, n2);
                    this.g(n3, n4 + 1, n2);
                    if (!bl2) continue;
                    this.e.C.b();
                    return;
                }
            }
            this.w = false;
        }
        this.e.C.b();
    }

    private void g(int n2, int n3, int n4) {
        int n5 = this.e.f(n2, n3);
        if (n5 > n4) {
            this.c(n2, n3, n4, n5 + 1);
        } else if (n5 < n4) {
            this.c(n2, n3, n5, n4 + 1);
        }
    }

    private void c(int n2, int n3, int n4, int n5) {
        if (n5 > n4 && this.e.a(n2, 0, n3, 16)) {
            for (int i2 = n4; i2 < n5; ++i2) {
                this.e.c(ahn.a, n2, i2, n3);
            }
            this.n = true;
        }
    }

    private void h(int n2, int n3, int n4) {
        int n5;
        int n6;
        int n7;
        int n8;
        int n9 = n8 = this.f[n4 << 4 | n2] & 0xFF;
        if (n3 > n8) {
            n9 = n3;
        }
        while (n9 > 0 && this.b(n2, n9 - 1, n4) == 0) {
            --n9;
        }
        if (n9 == n8) {
            return;
        }
        this.e.b(n2 + this.g * 16, n4 + this.h * 16, n9, n8);
        this.f[n4 << 4 | n2] = n9;
        int n10 = this.g * 16 + n2;
        int n11 = this.h * 16 + n4;
        if (!this.e.t.g) {
            apz apz2;
            if (n9 < n8) {
                for (n7 = n9; n7 < n8; ++n7) {
                    apz2 = this.u[n7 >> 4];
                    if (apz2 == null) continue;
                    apz2.b(n2, n7 & 0xF, n4, 15);
                    this.e.m((this.g << 4) + n2, n7, (this.h << 4) + n4);
                }
            } else {
                for (n7 = n8; n7 < n9; ++n7) {
                    apz2 = this.u[n7 >> 4];
                    if (apz2 == null) continue;
                    apz2.b(n2, n7 & 0xF, n4, 0);
                    this.e.m((this.g << 4) + n2, n7, (this.h << 4) + n4);
                }
            }
            n7 = 15;
            while (n9 > 0 && n7 > 0) {
                apz apz3;
                int n12;
                if ((n12 = this.b(n2, --n9, n4)) == 0) {
                    n12 = 1;
                }
                if ((n7 -= n12) < 0) {
                    n7 = 0;
                }
                if ((apz3 = this.u[n9 >> 4]) == null) continue;
                apz3.b(n2, n9 & 0xF, n4, n7);
            }
        }
        if ((n6 = (n7 = this.f[n4 << 4 | n2])) < (n5 = n8)) {
            int n13 = n5;
            n5 = n6;
            n6 = n13;
        }
        if (n7 < this.r) {
            this.r = n7;
        }
        if (!this.e.t.g) {
            this.c(n10 - 1, n11, n5, n6);
            this.c(n10 + 1, n11, n5, n6);
            this.c(n10, n11 - 1, n5, n6);
            this.c(n10, n11 + 1, n5, n6);
            this.c(n10, n11, n5, n6);
        }
        this.n = true;
    }

    public int b(int n2, int n3, int n4) {
        return this.a(n2, n3, n4).k();
    }

    public aji a(int n2, int n3, int n4) {
        apz apz2;
        aji aji2 = ajn.a;
        if (n3 >> 4 < this.u.length && (apz2 = this.u[n3 >> 4]) != null) {
            try {
                aji2 = apz2.a(n2, n3 & 0xF, n4);
            }
            catch (Throwable throwable) {
                b b2 = b.a(throwable, "Getting block");
                k k2 = b2.a("Block being got");
                k2.a("Location", new apy(this, n2, n3, n4));
                throw new s(b2);
            }
        }
        return aji2;
    }

    public int c(int n2, int n3, int n4) {
        if (n3 >> 4 >= this.u.length) {
            return 0;
        }
        apz apz2 = this.u[n3 >> 4];
        if (apz2 != null) {
            return apz2.b(n2, n3 & 0xF, n4);
        }
        return 0;
    }

    public boolean a(int n2, int n3, int n4, aji aji2, int n5) {
        aor aor2;
        int n6 = n4 << 4 | n2;
        if (n3 >= this.b[n6] - 1) {
            this.b[n6] = -999;
        }
        int n7 = this.f[n6];
        aji aji3 = this.a(n2, n3, n4);
        int n8 = this.c(n2, n3, n4);
        if (aji3 == aji2 && n8 == n5) {
            return false;
        }
        apz apz2 = this.u[n3 >> 4];
        boolean bl2 = false;
        if (apz2 == null) {
            if (aji2 == ajn.a) {
                return false;
            }
            apz apz3 = new apz(n3 >> 4 << 4, !this.e.t.g);
            this.u[n3 >> 4] = apz3;
            apz2 = apz3;
            bl2 = n3 >= n7;
        }
        int n9 = this.g * 16 + n2;
        int n10 = this.h * 16 + n4;
        if (!this.e.E) {
            aji3.f(this.e, n9, n3, n10, n8);
        }
        apz2.a(n2, n3 & 0xF, n4, aji2);
        if (!this.e.E) {
            aji3.a(this.e, n9, n3, n10, aji3, n8);
        } else if (aji3 instanceof akw && aji3 != aji2) {
            this.e.p(n9, n3, n10);
        }
        if (apz2.a(n2, n3 & 0xF, n4) != aji2) {
            return false;
        }
        apz2.a(n2, n3 & 0xF, n4, n5);
        if (bl2) {
            this.b();
        } else {
            int n11 = aji2.k();
            int n12 = aji3.k();
            if (n11 > 0) {
                if (n3 >= n7) {
                    this.h(n2, n3 + 1, n4);
                }
            } else if (n3 == n7 - 1) {
                this.h(n2, n3, n4);
            }
            if (n11 != n12 && (n11 < n12 || this.a(ahn.a, n2, n3, n4) > 0 || this.a(ahn.b, n2, n3, n4) > 0)) {
                this.e(n2, n4);
            }
        }
        if (aji3 instanceof akw && (aor2 = this.e(n2, n3, n4)) != null) {
            aor2.u();
        }
        if (!this.e.E) {
            aji2.b(this.e, n9, n3, n10);
        }
        if (aji2 instanceof akw) {
            aor aor3 = this.e(n2, n3, n4);
            if (aor3 == null) {
                aor3 = ((akw)((Object)aji2)).a(this.e, n5);
                this.e.a(n9, n3, n10, aor3);
            }
            if (aor3 != null) {
                aor3.u();
            }
        }
        this.n = true;
        return true;
    }

    public boolean a(int n2, int n3, int n4, int n5) {
        aor aor2;
        apz apz2 = this.u[n3 >> 4];
        if (apz2 == null) {
            return false;
        }
        int n6 = apz2.b(n2, n3 & 0xF, n4);
        if (n6 == n5) {
            return false;
        }
        this.n = true;
        apz2.a(n2, n3 & 0xF, n4, n5);
        if (apz2.a(n2, n3 & 0xF, n4) instanceof akw && (aor2 = this.e(n2, n3, n4)) != null) {
            aor2.u();
            aor2.g = n5;
        }
        return true;
    }

    public int a(ahn ahn2, int n2, int n3, int n4) {
        apz apz2 = this.u[n3 >> 4];
        if (apz2 == null) {
            if (this.d(n2, n3, n4)) {
                return ahn2.c;
            }
            return 0;
        }
        if (ahn2 == ahn.a) {
            if (this.e.t.g) {
                return 0;
            }
            return apz2.c(n2, n3 & 0xF, n4);
        }
        if (ahn2 == ahn.b) {
            return apz2.d(n2, n3 & 0xF, n4);
        }
        return ahn2.c;
    }

    public void a(ahn ahn2, int n2, int n3, int n4, int n5) {
        apz apz2 = this.u[n3 >> 4];
        if (apz2 == null) {
            apz apz3 = new apz(n3 >> 4 << 4, !this.e.t.g);
            this.u[n3 >> 4] = apz3;
            apz2 = apz3;
            this.b();
        }
        this.n = true;
        if (ahn2 == ahn.a) {
            if (!this.e.t.g) {
                apz2.b(n2, n3 & 0xF, n4, n5);
            }
        } else if (ahn2 == ahn.b) {
            apz2.c(n2, n3 & 0xF, n4, n5);
        }
    }

    public int b(int n2, int n3, int n4, int n5) {
        int n6;
        int n7;
        apz apz2 = this.u[n3 >> 4];
        if (apz2 == null) {
            if (!this.e.t.g && n5 < ahn.a.c) {
                return ahn.a.c - n5;
            }
            return 0;
        }
        int n8 = n7 = this.e.t.g ? 0 : apz2.c(n2, n3 & 0xF, n4);
        if (n7 > 0) {
            a = true;
        }
        if ((n6 = apz2.d(n2, n3 & 0xF, n4)) > (n7 -= n5)) {
            n7 = n6;
        }
        return n7;
    }

    public void a(sa sa2) {
        int n2;
        this.o = true;
        int n3 = qh.c(sa2.s / 16.0);
        int n4 = qh.c(sa2.u / 16.0);
        if (n3 != this.g || n4 != this.h) {
            t.warn("Wrong location! " + sa2 + " (at " + n3 + ", " + n4 + " instead of " + this.g + ", " + this.h + ")");
            Thread.dumpStack();
        }
        if ((n2 = qh.c(sa2.t / 16.0)) < 0) {
            n2 = 0;
        }
        if (n2 >= this.j.length) {
            n2 = this.j.length - 1;
        }
        sa2.ag = true;
        sa2.ah = this.g;
        sa2.ai = n2;
        sa2.aj = this.h;
        this.j[n2].add(sa2);
    }

    public void b(sa sa2) {
        this.a(sa2, sa2.ai);
    }

    public void a(sa sa2, int n2) {
        if (n2 < 0) {
            n2 = 0;
        }
        if (n2 >= this.j.length) {
            n2 = this.j.length - 1;
        }
        this.j[n2].remove(sa2);
    }

    public boolean d(int n2, int n3, int n4) {
        return n3 >= this.f[n4 << 4 | n2];
    }

    public aor e(int n2, int n3, int n4) {
        agt agt2 = new agt(n2, n3, n4);
        aor aor2 = (aor)this.i.get(agt2);
        if (aor2 == null) {
            aji aji2 = this.a(n2, n3, n4);
            if (!aji2.u()) {
                return null;
            }
            aor2 = ((akw)((Object)aji2)).a(this.e, this.c(n2, n3, n4));
            this.e.a(this.g * 16 + n2, n3, this.h * 16 + n4, aor2);
        }
        if (aor2 != null && aor2.r()) {
            this.i.remove(agt2);
            return null;
        }
        return aor2;
    }

    public void a(aor aor2) {
        int n2 = aor2.c - this.g * 16;
        int n3 = aor2.d;
        int n4 = aor2.e - this.h * 16;
        this.a(n2, n3, n4, aor2);
        if (this.d) {
            this.e.g.add(aor2);
        }
    }

    public void a(int n2, int n3, int n4, aor aor2) {
        agt agt2 = new agt(n2, n3, n4);
        aor2.a(this.e);
        aor2.c = this.g * 16 + n2;
        aor2.d = n3;
        aor2.e = this.h * 16 + n4;
        if (!(this.a(n2, n3, n4) instanceof akw)) {
            return;
        }
        if (this.i.containsKey(agt2)) {
            ((aor)this.i.get(agt2)).s();
        }
        aor2.t();
        this.i.put(agt2, aor2);
    }

    public void f(int n2, int n3, int n4) {
        aor aor2;
        agt agt2 = new agt(n2, n3, n4);
        if (this.d && (aor2 = (aor)this.i.remove(agt2)) != null) {
            aor2.s();
        }
    }

    public void c() {
        this.d = true;
        this.e.a(this.i.values());
        for (int i2 = 0; i2 < this.j.length; ++i2) {
            for (sa sa2 : this.j[i2]) {
                sa2.X();
            }
            this.e.a(this.j[i2]);
        }
    }

    public void d() {
        this.d = false;
        for (aor aor2 : this.i.values()) {
            this.e.a(aor2);
        }
        for (int i2 = 0; i2 < this.j.length; ++i2) {
            this.e.b(this.j[i2]);
        }
    }

    public void e() {
        this.n = true;
    }

    public void a(sa sa2, azt azt2, List list, sj sj2) {
        int n2 = qh.c((azt2.b - 2.0) / 16.0);
        int n3 = qh.c((azt2.e + 2.0) / 16.0);
        n2 = qh.a(n2, 0, this.j.length - 1);
        n3 = qh.a(n3, 0, this.j.length - 1);
        for (int i2 = n2; i2 <= n3; ++i2) {
            List list2 = this.j[i2];
            for (int i3 = 0; i3 < list2.size(); ++i3) {
                sa sa3 = (sa)list2.get(i3);
                if (sa3 == sa2 || !sa3.C.b(azt2) || sj2 != null && !sj2.a(sa3)) continue;
                list.add(sa3);
                sa[] saArray = sa3.at();
                if (saArray == null) continue;
                for (int i4 = 0; i4 < saArray.length; ++i4) {
                    sa3 = saArray[i4];
                    if (sa3 == sa2 || !sa3.C.b(azt2) || sj2 != null && !sj2.a(sa3)) continue;
                    list.add(sa3);
                }
            }
        }
    }

    public void a(Class clazz, azt azt2, List list, sj sj2) {
        int n2 = qh.c((azt2.b - 2.0) / 16.0);
        int n3 = qh.c((azt2.e + 2.0) / 16.0);
        n2 = qh.a(n2, 0, this.j.length - 1);
        n3 = qh.a(n3, 0, this.j.length - 1);
        for (int i2 = n2; i2 <= n3; ++i2) {
            List list2 = this.j[i2];
            for (int i3 = 0; i3 < list2.size(); ++i3) {
                sa sa2 = (sa)list2.get(i3);
                if (!clazz.isAssignableFrom(sa2.getClass()) || !sa2.C.b(azt2) || sj2 != null && !sj2.a(sa2)) continue;
                list.add(sa2);
            }
        }
    }

    public boolean a(boolean bl2) {
        if (bl2 ? this.o && this.e.I() != this.p || this.n : this.o && this.e.I() >= this.p + 600L) {
            return true;
        }
        return this.n;
    }

    public Random a(long l2) {
        return new Random(this.e.H() + (long)(this.g * this.g * 4987142) + (long)(this.g * 5947611) + (long)(this.h * this.h) * 4392871L + (long)(this.h * 389711) ^ l2);
    }

    public boolean g() {
        return false;
    }

    public void a(apu apu2, apu apu3, int n2, int n3) {
        if (!this.k && apu2.a(n2 + 1, n3 + 1) && apu2.a(n2, n3 + 1) && apu2.a(n2 + 1, n3)) {
            apu2.a(apu3, n2, n3);
        }
        if (apu2.a(n2 - 1, n3) && !apu2.d((int)(n2 - 1), (int)n3).k && apu2.a(n2 - 1, n3 + 1) && apu2.a(n2, n3 + 1) && apu2.a(n2 - 1, n3 + 1)) {
            apu2.a(apu3, n2 - 1, n3);
        }
        if (apu2.a(n2, n3 - 1) && !apu2.d((int)n2, (int)(n3 - 1)).k && apu2.a(n2 + 1, n3 - 1) && apu2.a(n2 + 1, n3 - 1) && apu2.a(n2 + 1, n3)) {
            apu2.a(apu3, n2, n3 - 1);
        }
        if (apu2.a(n2 - 1, n3 - 1) && !apu2.d((int)(n2 - 1), (int)(n3 - 1)).k && apu2.a(n2, n3 - 1) && apu2.a(n2 - 1, n3)) {
            apu2.a(apu3, n2 - 1, n3 - 1);
        }
    }

    public int d(int n2, int n3) {
        int n4 = n2 | n3 << 4;
        int n5 = this.b[n4];
        if (n5 == -999) {
            int n6 = this.h() + 15;
            n5 = -1;
            while (n6 > 0 && n5 == -1) {
                aji aji2 = this.a(n2, n6, n3);
                awt awt2 = aji2.o();
                if (!awt2.c() && !awt2.d()) {
                    --n6;
                    continue;
                }
                n5 = n6 + 1;
            }
            this.b[n4] = n5;
        }
        return n5;
    }

    public void b(boolean bl2) {
        if (this.w && !this.e.t.g && !bl2) {
            this.c(this.e.E);
        }
        this.m = true;
        if (!this.l && this.k) {
            this.p();
        }
    }

    public boolean k() {
        return this.m && this.k && this.l;
    }

    public agu l() {
        return new agu(this.g, this.h);
    }

    public boolean c(int n2, int n3) {
        if (n2 < 0) {
            n2 = 0;
        }
        if (n3 >= 256) {
            n3 = 255;
        }
        for (int i2 = n2; i2 <= n3; i2 += 16) {
            apz apz2 = this.u[i2 >> 4];
            if (apz2 == null || apz2.a()) continue;
            return false;
        }
        return true;
    }

    public void a(apz[] apzArray) {
        this.u = apzArray;
    }

    public ahu a(int n2, int n3, aib aib2) {
        int n4 = this.v[n3 << 4 | n2] & 0xFF;
        if (n4 == 255) {
            ahu ahu2 = aib2.a((this.g << 4) + n2, (this.h << 4) + n3);
            n4 = ahu2.ay;
            this.v[n3 << 4 | n2] = (byte)(n4 & 0xFF);
        }
        if (ahu.d(n4) == null) {
            return ahu.p;
        }
        return ahu.d(n4);
    }

    public byte[] m() {
        return this.v;
    }

    public void a(byte[] byArray) {
        this.v = byArray;
    }

    public void n() {
        this.x = 0;
    }

    public void o() {
        for (int i2 = 0; i2 < 8; ++i2) {
            if (this.x >= 4096) {
                return;
            }
            int n2 = this.x % 16;
            int n3 = this.x / 16 % 16;
            int n4 = this.x / 256;
            ++this.x;
            int n5 = (this.g << 4) + n3;
            int n6 = (this.h << 4) + n4;
            for (int i3 = 0; i3 < 16; ++i3) {
                int n7 = (n2 << 4) + i3;
                if ((this.u[n2] != null || i3 != 0 && i3 != 15 && n3 != 0 && n3 != 15 && n4 != 0 && n4 != 15) && (this.u[n2] == null || this.u[n2].a(n3, i3, n4).o() != awt.a)) continue;
                if (this.e.a(n5, n7 - 1, n6).m() > 0) {
                    this.e.t(n5, n7 - 1, n6);
                }
                if (this.e.a(n5, n7 + 1, n6).m() > 0) {
                    this.e.t(n5, n7 + 1, n6);
                }
                if (this.e.a(n5 - 1, n7, n6).m() > 0) {
                    this.e.t(n5 - 1, n7, n6);
                }
                if (this.e.a(n5 + 1, n7, n6).m() > 0) {
                    this.e.t(n5 + 1, n7, n6);
                }
                if (this.e.a(n5, n7, n6 - 1).m() > 0) {
                    this.e.t(n5, n7, n6 - 1);
                }
                if (this.e.a(n5, n7, n6 + 1).m() > 0) {
                    this.e.t(n5, n7, n6 + 1);
                }
                this.e.t(n5, n7, n6);
            }
        }
    }

    public void p() {
        this.k = true;
        this.l = true;
        if (!this.e.t.g) {
            if (this.e.b(this.g * 16 - 1, 0, this.h * 16 - 1, this.g * 16 + 1, 63, this.h * 16 + 1)) {
                block0: for (int i2 = 0; i2 < 16; ++i2) {
                    for (int i3 = 0; i3 < 16; ++i3) {
                        if (this.f(i2, i3)) continue;
                        this.l = false;
                        continue block0;
                    }
                }
                if (this.l) {
                    apx apx2 = this.e.d(this.g * 16 - 1, this.h * 16);
                    apx2.a(3);
                    apx2 = this.e.d(this.g * 16 + 16, this.h * 16);
                    apx2.a(1);
                    apx2 = this.e.d(this.g * 16, this.h * 16 - 1);
                    apx2.a(0);
                    apx2 = this.e.d(this.g * 16, this.h * 16 + 16);
                    apx2.a(2);
                }
            } else {
                this.l = false;
            }
        }
    }

    private void a(int n2) {
        block6: {
            block8: {
                block7: {
                    block5: {
                        if (!this.k) {
                            return;
                        }
                        if (n2 != 3) break block5;
                        for (int i2 = 0; i2 < 16; ++i2) {
                            this.f(15, i2);
                        }
                        break block6;
                    }
                    if (n2 != 1) break block7;
                    for (int i3 = 0; i3 < 16; ++i3) {
                        this.f(0, i3);
                    }
                    break block6;
                }
                if (n2 != 0) break block8;
                for (int i4 = 0; i4 < 16; ++i4) {
                    this.f(i4, 15);
                }
                break block6;
            }
            if (n2 != 2) break block6;
            for (int i5 = 0; i5 < 16; ++i5) {
                this.f(i5, 0);
            }
        }
    }

    private boolean f(int n2, int n3) {
        int n4;
        int n5 = this.h();
        boolean bl2 = false;
        boolean bl3 = false;
        for (n4 = n5 + 16 - 1; n4 > 63 || n4 > 0 && !bl3; --n4) {
            int n6 = this.b(n2, n4, n3);
            if (n6 == 255 && n4 < 63) {
                bl3 = true;
            }
            if (!bl2 && n6 > 0) {
                bl2 = true;
                continue;
            }
            if (!bl2 || n6 != 0 || this.e.t(this.g * 16 + n2, n4, this.h * 16 + n3)) continue;
            return false;
        }
        while (n4 > 0) {
            if (this.a(n2, n4, n3).m() > 0) {
                this.e.t(this.g * 16 + n2, n4, this.h * 16 + n3);
            }
            --n4;
        }
        return true;
    }
}

