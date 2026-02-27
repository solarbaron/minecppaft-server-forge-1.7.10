/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Collection;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Random;
import java.util.Set;
import java.util.UUID;
import net.minecraft.server.MinecraftServer;

public abstract class ahb
implements ahl {
    public boolean d;
    public List e = new ArrayList();
    protected List f = new ArrayList();
    public List g = new ArrayList();
    private List a = new ArrayList();
    private List b = new ArrayList();
    public List h = new ArrayList();
    public List i = new ArrayList();
    private long c = 0xFFFFFFL;
    public int j;
    protected int k = new Random().nextInt();
    protected final int l = 1013904223;
    protected float m;
    protected float n;
    protected float o;
    protected float p;
    public int q;
    public rd r;
    public Random s = new Random();
    public final aqo t;
    protected List u = new ArrayList();
    protected apu v;
    protected final azc w;
    protected ays x;
    public boolean y;
    public azq z;
    public final wc A;
    protected final wb B = new wb(this);
    public final qi C;
    private final Calendar J = Calendar.getInstance();
    protected bac D = new bac();
    public boolean E;
    protected Set F = new HashSet();
    private int K = this.s.nextInt(12000);
    protected boolean G = true;
    protected boolean H = true;
    private ArrayList L = new ArrayList();
    private boolean M;
    int[] I = new int[32768];

    public ahu a(int n2, int n3) {
        if (this.d(n2, 0, n3)) {
            apx apx2 = this.d(n2, n3);
            try {
                return apx2.a(n2 & 0xF, n3 & 0xF, this.t.e);
            }
            catch (Throwable throwable) {
                b b2 = b.a(throwable, "Getting biome");
                k k2 = b2.a("Coordinates of biome request");
                k2.a("Location", new ahc(this, n2, n3));
                throw new s(b2);
            }
        }
        return this.t.e.a(n2, n3);
    }

    public aib v() {
        return this.t.e;
    }

    public ahb(azc azc2, String string, ahj ahj2, aqo aqo2, qi qi2) {
        wc wc2;
        this.w = azc2;
        this.C = qi2;
        this.z = new azq(azc2);
        this.x = azc2.d();
        this.t = aqo2 != null ? aqo2 : (this.x != null && this.x.j() != 0 ? aqo.a(this.x.j()) : aqo.a(0));
        if (this.x == null) {
            this.x = new ays(ahj2, string);
        } else {
            this.x.a(string);
        }
        this.t.a(this);
        this.v = this.j();
        if (!this.x.w()) {
            try {
                this.a(ahj2);
            }
            catch (Throwable throwable) {
                b b2 = b.a(throwable, "Exception initializing level");
                try {
                    this.a(b2);
                }
                catch (Throwable throwable2) {
                    // empty catch block
                }
                throw new s(b2);
            }
            this.x.d(true);
        }
        if ((wc2 = (wc)this.z.a(wc.class, "villages")) == null) {
            this.A = new wc(this);
            this.z.a("villages", this.A);
        } else {
            this.A = wc2;
            this.A.a(this);
        }
        this.B();
        this.a();
    }

    protected abstract apu j();

    protected void a(ahj ahj2) {
        this.x.d(true);
    }

    public aji b(int n2, int n3) {
        int n4 = 63;
        while (!this.c(n2, n4 + 1, n3)) {
            ++n4;
        }
        return this.a(n2, n4, n3);
    }

    @Override
    public aji a(int n2, int n3, int n4) {
        if (n2 < -30000000 || n4 < -30000000 || n2 >= 30000000 || n4 >= 30000000 || n3 < 0 || n3 >= 256) {
            return ajn.a;
        }
        apx apx2 = null;
        try {
            apx2 = this.e(n2 >> 4, n4 >> 4);
            return apx2.a(n2 & 0xF, n3, n4 & 0xF);
        }
        catch (Throwable throwable) {
            b b2 = b.a(throwable, "Exception getting block type in world");
            k k2 = b2.a("Requested block coordinates");
            k2.a("Found chunk", apx2 == null);
            k2.a("Location", k.a(n2, n3, n4));
            throw new s(b2);
        }
    }

    public boolean c(int n2, int n3, int n4) {
        return this.a(n2, n3, n4).o() == awt.a;
    }

    public boolean d(int n2, int n3, int n4) {
        if (n3 < 0 || n3 >= 256) {
            return false;
        }
        return this.c(n2 >> 4, n4 >> 4);
    }

    public boolean a(int n2, int n3, int n4, int n5) {
        return this.b(n2 - n5, n3 - n5, n4 - n5, n2 + n5, n3 + n5, n4 + n5);
    }

    public boolean b(int n2, int n3, int n4, int n5, int n6, int n7) {
        if (n6 < 0 || n3 >= 256) {
            return false;
        }
        n4 >>= 4;
        n5 >>= 4;
        n7 >>= 4;
        for (int i2 = n2 >>= 4; i2 <= n5; ++i2) {
            for (int i3 = n4; i3 <= n7; ++i3) {
                if (this.c(i2, i3)) continue;
                return false;
            }
        }
        return true;
    }

    protected boolean c(int n2, int n3) {
        return this.v.a(n2, n3);
    }

    public apx d(int n2, int n3) {
        return this.e(n2 >> 4, n3 >> 4);
    }

    public apx e(int n2, int n3) {
        return this.v.d(n2, n3);
    }

    public boolean d(int n2, int n3, int n4, aji aji2, int n5, int n6) {
        if (n2 < -30000000 || n4 < -30000000 || n2 >= 30000000 || n4 >= 30000000) {
            return false;
        }
        if (n3 < 0) {
            return false;
        }
        if (n3 >= 256) {
            return false;
        }
        apx apx2 = this.e(n2 >> 4, n4 >> 4);
        aji aji3 = null;
        if ((n6 & 1) != 0) {
            aji3 = apx2.a(n2 & 0xF, n3, n4 & 0xF);
        }
        boolean bl2 = apx2.a(n2 & 0xF, n3, n4 & 0xF, aji2, n5);
        this.C.a("checkLight");
        this.t(n2, n3, n4);
        this.C.b();
        if (bl2) {
            if ((n6 & 2) != 0 && (!this.E || (n6 & 4) == 0) && apx2.k()) {
                this.g(n2, n3, n4);
            }
            if (!this.E && (n6 & 1) != 0) {
                this.c(n2, n3, n4, aji3);
                if (aji2.M()) {
                    this.f(n2, n3, n4, aji2);
                }
            }
        }
        return bl2;
    }

    @Override
    public int e(int n2, int n3, int n4) {
        if (n2 < -30000000 || n4 < -30000000 || n2 >= 30000000 || n4 >= 30000000) {
            return 0;
        }
        if (n3 < 0) {
            return 0;
        }
        if (n3 >= 256) {
            return 0;
        }
        apx apx2 = this.e(n2 >> 4, n4 >> 4);
        return apx2.c(n2 &= 0xF, n3, n4 &= 0xF);
    }

    public boolean a(int n2, int n3, int n4, int n5, int n6) {
        int n7;
        int n8;
        if (n2 < -30000000 || n4 < -30000000 || n2 >= 30000000 || n4 >= 30000000) {
            return false;
        }
        if (n3 < 0) {
            return false;
        }
        if (n3 >= 256) {
            return false;
        }
        apx apx2 = this.e(n2 >> 4, n4 >> 4);
        boolean bl2 = apx2.a(n8 = n2 & 0xF, n3, n7 = n4 & 0xF, n5);
        if (bl2) {
            aji aji2 = apx2.a(n8, n3, n7);
            if ((n6 & 2) != 0 && (!this.E || (n6 & 4) == 0) && apx2.k()) {
                this.g(n2, n3, n4);
            }
            if (!this.E && (n6 & 1) != 0) {
                this.c(n2, n3, n4, aji2);
                if (aji2.M()) {
                    this.f(n2, n3, n4, aji2);
                }
            }
        }
        return bl2;
    }

    public boolean f(int n2, int n3, int n4) {
        return this.d(n2, n3, n4, ajn.a, 0, 3);
    }

    public boolean a(int n2, int n3, int n4, boolean bl2) {
        aji aji2 = this.a(n2, n3, n4);
        if (aji2.o() == awt.a) {
            return false;
        }
        int n5 = this.e(n2, n3, n4);
        this.c(2001, n2, n3, n4, aji.b(aji2) + (n5 << 12));
        if (bl2) {
            aji2.b(this, n2, n3, n4, n5, 0);
        }
        return this.d(n2, n3, n4, ajn.a, 0, 3);
    }

    public boolean b(int n2, int n3, int n4, aji aji2) {
        return this.d(n2, n3, n4, aji2, 0, 3);
    }

    public void g(int n2, int n3, int n4) {
        for (int i2 = 0; i2 < this.u.size(); ++i2) {
            ((ahh)this.u.get(i2)).a(n2, n3, n4);
        }
    }

    public void c(int n2, int n3, int n4, aji aji2) {
        this.d(n2, n3, n4, aji2);
    }

    public void b(int n2, int n3, int n4, int n5) {
        int n6;
        if (n4 > n5) {
            n6 = n5;
            n5 = n4;
            n4 = n6;
        }
        if (!this.t.g) {
            for (n6 = n4; n6 <= n5; ++n6) {
                this.c(ahn.a, n2, n6, n3);
            }
        }
        this.c(n2, n4, n3, n2, n5, n3);
    }

    public void c(int n2, int n3, int n4, int n5, int n6, int n7) {
        for (int i2 = 0; i2 < this.u.size(); ++i2) {
            ((ahh)this.u.get(i2)).a(n2, n3, n4, n5, n6, n7);
        }
    }

    public void d(int n2, int n3, int n4, aji aji2) {
        this.e(n2 - 1, n3, n4, aji2);
        this.e(n2 + 1, n3, n4, aji2);
        this.e(n2, n3 - 1, n4, aji2);
        this.e(n2, n3 + 1, n4, aji2);
        this.e(n2, n3, n4 - 1, aji2);
        this.e(n2, n3, n4 + 1, aji2);
    }

    public void b(int n2, int n3, int n4, aji aji2, int n5) {
        if (n5 != 4) {
            this.e(n2 - 1, n3, n4, aji2);
        }
        if (n5 != 5) {
            this.e(n2 + 1, n3, n4, aji2);
        }
        if (n5 != 0) {
            this.e(n2, n3 - 1, n4, aji2);
        }
        if (n5 != 1) {
            this.e(n2, n3 + 1, n4, aji2);
        }
        if (n5 != 2) {
            this.e(n2, n3, n4 - 1, aji2);
        }
        if (n5 != 3) {
            this.e(n2, n3, n4 + 1, aji2);
        }
    }

    public void e(int n2, int n3, int n4, aji aji2) {
        if (this.E) {
            return;
        }
        aji aji3 = this.a(n2, n3, n4);
        try {
            aji3.a(this, n2, n3, n4, aji2);
        }
        catch (Throwable throwable) {
            int n5;
            b b2 = b.a(throwable, "Exception while updating neighbours");
            k k2 = b2.a("Block being updated");
            try {
                n5 = this.e(n2, n3, n4);
            }
            catch (Throwable throwable2) {
                n5 = -1;
            }
            k2.a("Source block type", new ahd(this, aji2));
            k.a(k2, n2, n3, n4, aji3, n5);
            throw new s(b2);
        }
    }

    public boolean a(int n2, int n3, int n4, aji aji2) {
        return false;
    }

    public boolean i(int n2, int n3, int n4) {
        return this.e(n2 >> 4, n4 >> 4).d(n2 & 0xF, n3, n4 & 0xF);
    }

    public int j(int n2, int n3, int n4) {
        if (n3 < 0) {
            return 0;
        }
        if (n3 >= 256) {
            n3 = 255;
        }
        return this.e(n2 >> 4, n4 >> 4).b(n2 & 0xF, n3, n4 & 0xF, 0);
    }

    public int k(int n2, int n3, int n4) {
        return this.b(n2, n3, n4, true);
    }

    public int b(int n2, int n3, int n4, boolean bl2) {
        if (n2 < -30000000 || n4 < -30000000 || n2 >= 30000000 || n4 >= 30000000) {
            return 15;
        }
        if (bl2 && this.a(n2, n3, n4).n()) {
            int n5 = this.b(n2, n3 + 1, n4, false);
            int n6 = this.b(n2 + 1, n3, n4, false);
            int n7 = this.b(n2 - 1, n3, n4, false);
            int n8 = this.b(n2, n3, n4 + 1, false);
            int n9 = this.b(n2, n3, n4 - 1, false);
            if (n6 > n5) {
                n5 = n6;
            }
            if (n7 > n5) {
                n5 = n7;
            }
            if (n8 > n5) {
                n5 = n8;
            }
            if (n9 > n5) {
                n5 = n9;
            }
            return n5;
        }
        if (n3 < 0) {
            return 0;
        }
        if (n3 >= 256) {
            n3 = 255;
        }
        apx apx2 = this.e(n2 >> 4, n4 >> 4);
        return apx2.b(n2 &= 0xF, n3, n4 &= 0xF, this.j);
    }

    public int f(int n2, int n3) {
        if (n2 < -30000000 || n3 < -30000000 || n2 >= 30000000 || n3 >= 30000000) {
            return 64;
        }
        if (!this.c(n2 >> 4, n3 >> 4)) {
            return 0;
        }
        apx apx2 = this.e(n2 >> 4, n3 >> 4);
        return apx2.b(n2 & 0xF, n3 & 0xF);
    }

    public int g(int n2, int n3) {
        if (n2 < -30000000 || n3 < -30000000 || n2 >= 30000000 || n3 >= 30000000) {
            return 64;
        }
        if (!this.c(n2 >> 4, n3 >> 4)) {
            return 0;
        }
        apx apx2 = this.e(n2 >> 4, n3 >> 4);
        return apx2.r;
    }

    public int b(ahn ahn2, int n2, int n3, int n4) {
        if (n3 < 0) {
            n3 = 0;
        }
        if (n3 >= 256) {
            n3 = 255;
        }
        if (n2 < -30000000 || n4 < -30000000 || n2 >= 30000000 || n4 >= 30000000) {
            return ahn2.c;
        }
        int n5 = n2 >> 4;
        int n6 = n4 >> 4;
        if (!this.c(n5, n6)) {
            return ahn2.c;
        }
        apx apx2 = this.e(n5, n6);
        return apx2.a(ahn2, n2 & 0xF, n3, n4 & 0xF);
    }

    public void b(ahn ahn2, int n2, int n3, int n4, int n5) {
        if (n2 < -30000000 || n4 < -30000000 || n2 >= 30000000 || n4 >= 30000000) {
            return;
        }
        if (n3 < 0) {
            return;
        }
        if (n3 >= 256) {
            return;
        }
        if (!this.c(n2 >> 4, n4 >> 4)) {
            return;
        }
        apx apx2 = this.e(n2 >> 4, n4 >> 4);
        apx2.a(ahn2, n2 & 0xF, n3, n4 & 0xF, n5);
        for (int i2 = 0; i2 < this.u.size(); ++i2) {
            ((ahh)this.u.get(i2)).b(n2, n3, n4);
        }
    }

    public void m(int n2, int n3, int n4) {
        for (int i2 = 0; i2 < this.u.size(); ++i2) {
            ((ahh)this.u.get(i2)).b(n2, n3, n4);
        }
    }

    public float n(int n2, int n3, int n4) {
        return this.t.h[this.k(n2, n3, n4)];
    }

    public boolean w() {
        return this.j < 4;
    }

    public azu a(azw azw2, azw azw3) {
        return this.a(azw2, azw3, false, false, false);
    }

    public azu a(azw azw2, azw azw3, boolean bl2) {
        return this.a(azw2, azw3, bl2, false, false);
    }

    public azu a(azw azw2, azw azw3, boolean bl2, boolean bl3, boolean bl4) {
        azu azu2;
        if (Double.isNaN(azw2.a) || Double.isNaN(azw2.b) || Double.isNaN(azw2.c)) {
            return null;
        }
        if (Double.isNaN(azw3.a) || Double.isNaN(azw3.b) || Double.isNaN(azw3.c)) {
            return null;
        }
        int n2 = qh.c(azw3.a);
        int n3 = qh.c(azw3.b);
        int n4 = qh.c(azw3.c);
        int n5 = qh.c(azw2.a);
        int n6 = qh.c(azw2.b);
        int n7 = qh.c(azw2.c);
        Object object = this.a(n5, n6, n7);
        int n8 = this.e(n5, n6, n7);
        if ((!bl3 || ((aji)object).a(this, n5, n6, n7) != null) && ((aji)object).a(n8, bl2) && (azu2 = ((aji)object).a(this, n5, n6, n7, azw2, azw3)) != null) {
            return azu2;
        }
        object = null;
        n8 = 200;
        while (n8-- >= 0) {
            if (Double.isNaN(azw2.a) || Double.isNaN(azw2.b) || Double.isNaN(azw2.c)) {
                return null;
            }
            if (n5 == n2 && n6 == n3 && n7 == n4) {
                return bl4 ? object : null;
            }
            boolean bl5 = true;
            boolean bl6 = true;
            boolean bl7 = true;
            double d2 = 999.0;
            double d3 = 999.0;
            double d4 = 999.0;
            if (n2 > n5) {
                d2 = (double)n5 + 1.0;
            } else if (n2 < n5) {
                d2 = (double)n5 + 0.0;
            } else {
                bl5 = false;
            }
            if (n3 > n6) {
                d3 = (double)n6 + 1.0;
            } else if (n3 < n6) {
                d3 = (double)n6 + 0.0;
            } else {
                bl6 = false;
            }
            if (n4 > n7) {
                d4 = (double)n7 + 1.0;
            } else if (n4 < n7) {
                d4 = (double)n7 + 0.0;
            } else {
                bl7 = false;
            }
            double d5 = 999.0;
            double d6 = 999.0;
            double d7 = 999.0;
            double d8 = azw3.a - azw2.a;
            double d9 = azw3.b - azw2.b;
            double d10 = azw3.c - azw2.c;
            if (bl5) {
                d5 = (d2 - azw2.a) / d8;
            }
            if (bl6) {
                d6 = (d3 - azw2.b) / d9;
            }
            if (bl7) {
                d7 = (d4 - azw2.c) / d10;
            }
            int n9 = 0;
            if (d5 < d6 && d5 < d7) {
                n9 = n2 > n5 ? 4 : 5;
                azw2.a = d2;
                azw2.b += d9 * d5;
                azw2.c += d10 * d5;
            } else if (d6 < d7) {
                n9 = n3 > n6 ? 0 : 1;
                azw2.a += d8 * d6;
                azw2.b = d3;
                azw2.c += d10 * d6;
            } else {
                n9 = n4 > n7 ? 2 : 3;
                azw2.a += d8 * d7;
                azw2.b += d9 * d7;
                azw2.c = d4;
            }
            azw azw4 = azw.a(azw2.a, azw2.b, azw2.c);
            azw4.a = qh.c(azw2.a);
            n5 = (int)azw4.a;
            if (n9 == 5) {
                --n5;
                azw4.a += 1.0;
            }
            azw4.b = qh.c(azw2.b);
            n6 = (int)azw4.b;
            if (n9 == 1) {
                --n6;
                azw4.b += 1.0;
            }
            azw4.c = qh.c(azw2.c);
            n7 = (int)azw4.c;
            if (n9 == 3) {
                --n7;
                azw4.c += 1.0;
            }
            aji aji2 = this.a(n5, n6, n7);
            int n10 = this.e(n5, n6, n7);
            if (bl3 && aji2.a(this, n5, n6, n7) == null) continue;
            if (aji2.a(n10, bl2)) {
                azu azu3 = aji2.a(this, n5, n6, n7, azw2, azw3);
                if (azu3 == null) continue;
                return azu3;
            }
            object = new azu(n5, n6, n7, n9, azw2, false);
        }
        return bl4 ? object : null;
    }

    public void a(sa sa2, String string, float f2, float f3) {
        for (int i2 = 0; i2 < this.u.size(); ++i2) {
            ((ahh)this.u.get(i2)).a(string, sa2.s, sa2.t - (double)sa2.L, sa2.u, f2, f3);
        }
    }

    public void a(yz yz2, String string, float f2, float f3) {
        for (int i2 = 0; i2 < this.u.size(); ++i2) {
            ((ahh)this.u.get(i2)).a(yz2, string, yz2.s, yz2.t - (double)yz2.L, yz2.u, f2, f3);
        }
    }

    public void a(double d2, double d3, double d4, String string, float f2, float f3) {
        for (int i2 = 0; i2 < this.u.size(); ++i2) {
            ((ahh)this.u.get(i2)).a(string, d2, d3, d4, f2, f3);
        }
    }

    public void a(double d2, double d3, double d4, String string, float f2, float f3, boolean bl2) {
    }

    public void a(String string, int n2, int n3, int n4) {
        for (int i2 = 0; i2 < this.u.size(); ++i2) {
            ((ahh)this.u.get(i2)).a(string, n2, n3, n4);
        }
    }

    public void a(String string, double d2, double d3, double d4, double d5, double d6, double d7) {
        for (int i2 = 0; i2 < this.u.size(); ++i2) {
            ((ahh)this.u.get(i2)).a(string, d2, d3, d4, d5, d6, d7);
        }
    }

    public boolean c(sa sa2) {
        this.i.add(sa2);
        return true;
    }

    public boolean d(sa sa2) {
        int n2 = qh.c(sa2.s / 16.0);
        int n3 = qh.c(sa2.u / 16.0);
        boolean bl2 = sa2.n;
        if (sa2 instanceof yz) {
            bl2 = true;
        }
        if (bl2 || this.c(n2, n3)) {
            if (sa2 instanceof yz) {
                yz yz2 = (yz)sa2;
                this.h.add(yz2);
                this.c();
            }
            this.e(n2, n3).a(sa2);
            this.e.add(sa2);
            this.a(sa2);
            return true;
        }
        return false;
    }

    protected void a(sa sa2) {
        for (int i2 = 0; i2 < this.u.size(); ++i2) {
            ((ahh)this.u.get(i2)).a(sa2);
        }
    }

    protected void b(sa sa2) {
        for (int i2 = 0; i2 < this.u.size(); ++i2) {
            ((ahh)this.u.get(i2)).b(sa2);
        }
    }

    public void e(sa sa2) {
        if (sa2.l != null) {
            sa2.l.a((sa)null);
        }
        if (sa2.m != null) {
            sa2.a((sa)null);
        }
        sa2.B();
        if (sa2 instanceof yz) {
            this.h.remove(sa2);
            this.c();
            this.b(sa2);
        }
    }

    public void f(sa sa2) {
        sa2.B();
        if (sa2 instanceof yz) {
            this.h.remove(sa2);
            this.c();
        }
        int n2 = sa2.ah;
        int n3 = sa2.aj;
        if (sa2.ag && this.c(n2, n3)) {
            this.e(n2, n3).b(sa2);
        }
        this.e.remove(sa2);
        this.b(sa2);
    }

    public void a(ahh ahh2) {
        this.u.add(ahh2);
    }

    public List a(sa sa2, azt azt2) {
        this.L.clear();
        int n2 = qh.c(azt2.a);
        int n3 = qh.c(azt2.d + 1.0);
        int n4 = qh.c(azt2.b);
        int n5 = qh.c(azt2.e + 1.0);
        int n6 = qh.c(azt2.c);
        int n7 = qh.c(azt2.f + 1.0);
        for (int i2 = n2; i2 < n3; ++i2) {
            for (int i3 = n6; i3 < n7; ++i3) {
                if (!this.d(i2, 64, i3)) continue;
                for (int i4 = n4 - 1; i4 < n5; ++i4) {
                    aji aji2 = i2 < -30000000 || i2 >= 30000000 || i3 < -30000000 || i3 >= 30000000 ? ajn.b : this.a(i2, i4, i3);
                    aji2.a(this, i2, i4, i3, azt2, this.L, sa2);
                }
            }
        }
        double d2 = 0.25;
        List list = this.b(sa2, azt2.b(d2, d2, d2));
        for (int i5 = 0; i5 < list.size(); ++i5) {
            azt azt3 = ((sa)list.get(i5)).J();
            if (azt3 != null && azt3.b(azt2)) {
                this.L.add(azt3);
            }
            if ((azt3 = sa2.h((sa)list.get(i5))) == null || !azt3.b(azt2)) continue;
            this.L.add(azt3);
        }
        return this.L;
    }

    public List a(azt azt2) {
        this.L.clear();
        int n2 = qh.c(azt2.a);
        int n3 = qh.c(azt2.d + 1.0);
        int n4 = qh.c(azt2.b);
        int n5 = qh.c(azt2.e + 1.0);
        int n6 = qh.c(azt2.c);
        int n7 = qh.c(azt2.f + 1.0);
        for (int i2 = n2; i2 < n3; ++i2) {
            for (int i3 = n6; i3 < n7; ++i3) {
                if (!this.d(i2, 64, i3)) continue;
                for (int i4 = n4 - 1; i4 < n5; ++i4) {
                    aji aji2 = i2 < -30000000 || i2 >= 30000000 || i3 < -30000000 || i3 >= 30000000 ? ajn.h : this.a(i2, i4, i3);
                    aji2.a(this, i2, i4, i3, azt2, this.L, null);
                }
            }
        }
        return this.L;
    }

    public int a(float f2) {
        float f3 = this.c(f2);
        float f4 = 1.0f - (qh.b(f3 * (float)Math.PI * 2.0f) * 2.0f + 0.5f);
        if (f4 < 0.0f) {
            f4 = 0.0f;
        }
        if (f4 > 1.0f) {
            f4 = 1.0f;
        }
        f4 = 1.0f - f4;
        f4 = (float)((double)f4 * (1.0 - (double)(this.j(f2) * 5.0f) / 16.0));
        f4 = (float)((double)f4 * (1.0 - (double)(this.h(f2) * 5.0f) / 16.0));
        f4 = 1.0f - f4;
        return (int)(f4 * 11.0f);
    }

    public float c(float f2) {
        return this.t.a(this.x.g(), f2);
    }

    public float y() {
        return aqo.a[this.t.a(this.x.g())];
    }

    public float d(float f2) {
        float f3 = this.c(f2);
        return f3 * (float)Math.PI * 2.0f;
    }

    public int h(int n2, int n3) {
        return this.d(n2, n3).d(n2 & 0xF, n3 & 0xF);
    }

    public int i(int n2, int n3) {
        apx apx2 = this.d(n2, n3);
        n2 &= 0xF;
        n3 &= 0xF;
        for (int i2 = apx2.h() + 15; i2 > 0; --i2) {
            aji aji2 = apx2.a(n2, i2, n3);
            if (!aji2.o().c() || aji2.o() == awt.j) {
                continue;
            }
            return i2 + 1;
        }
        return -1;
    }

    public void a(int n2, int n3, int n4, aji aji2, int n5) {
    }

    public void a(int n2, int n3, int n4, aji aji2, int n5, int n6) {
    }

    public void b(int n2, int n3, int n4, aji aji2, int n5, int n6) {
    }

    public void h() {
        int n2;
        int n3;
        Object object;
        int n4;
        this.C.a("entities");
        this.C.a("global");
        for (n4 = 0; n4 < this.i.size(); ++n4) {
            object = (sa)this.i.get(n4);
            try {
                ++((sa)object).aa;
                ((sa)object).h();
            }
            catch (Throwable throwable) {
                b b2 = b.a(throwable, "Ticking entity");
                k k2 = b2.a("Entity being ticked");
                if (object == null) {
                    k2.a("Entity", "~~NULL~~");
                } else {
                    ((sa)object).a(k2);
                }
                throw new s(b2);
            }
            if (!((sa)object).K) continue;
            this.i.remove(n4--);
        }
        this.C.c("remove");
        this.e.removeAll(this.f);
        for (n4 = 0; n4 < this.f.size(); ++n4) {
            object = (sa)this.f.get(n4);
            n3 = ((sa)object).ah;
            n2 = ((sa)object).aj;
            if (!((sa)object).ag || !this.c(n3, n2)) continue;
            this.e(n3, n2).b((sa)object);
        }
        for (n4 = 0; n4 < this.f.size(); ++n4) {
            this.b((sa)this.f.get(n4));
        }
        this.f.clear();
        this.C.c("regular");
        for (n4 = 0; n4 < this.e.size(); ++n4) {
            object = (sa)this.e.get(n4);
            if (((sa)object).m != null) {
                if (!((sa)object).m.K && ((sa)object).m.l == object) continue;
                ((sa)object).m.l = null;
                ((sa)object).m = null;
            }
            this.C.a("tick");
            if (!((sa)object).K) {
                try {
                    this.g((sa)object);
                }
                catch (Throwable throwable) {
                    b b3 = b.a(throwable, "Ticking entity");
                    k k3 = b3.a("Entity being ticked");
                    ((sa)object).a(k3);
                    throw new s(b3);
                }
            }
            this.C.b();
            this.C.a("remove");
            if (((sa)object).K) {
                n3 = ((sa)object).ah;
                n2 = ((sa)object).aj;
                if (((sa)object).ag && this.c(n3, n2)) {
                    this.e(n3, n2).b((sa)object);
                }
                this.e.remove(n4--);
                this.b((sa)object);
            }
            this.C.b();
        }
        this.C.c("blockEntities");
        this.M = true;
        Iterator iterator = this.g.iterator();
        while (iterator.hasNext()) {
            apx apx2;
            object = (aor)iterator.next();
            if (!((aor)object).r() && ((aor)object).o() && this.d(((aor)object).c, ((aor)object).d, ((aor)object).e)) {
                try {
                    ((aor)object).h();
                }
                catch (Throwable throwable) {
                    b b4 = b.a(throwable, "Ticking block entity");
                    k k4 = b4.a("Block entity being ticked");
                    ((aor)object).a(k4);
                    throw new s(b4);
                }
            }
            if (!((aor)object).r()) continue;
            iterator.remove();
            if (!this.c(((aor)object).c >> 4, ((aor)object).e >> 4) || (apx2 = this.e(((aor)object).c >> 4, ((aor)object).e >> 4)) == null) continue;
            apx2.f(((aor)object).c & 0xF, ((aor)object).d, ((aor)object).e & 0xF);
        }
        this.M = false;
        if (!this.b.isEmpty()) {
            this.g.removeAll(this.b);
            this.b.clear();
        }
        this.C.c("pendingBlockEntities");
        if (!this.a.isEmpty()) {
            for (int i2 = 0; i2 < this.a.size(); ++i2) {
                apx apx3;
                aor aor2 = (aor)this.a.get(i2);
                if (aor2.r()) continue;
                if (!this.g.contains(aor2)) {
                    this.g.add(aor2);
                }
                if (this.c(aor2.c >> 4, aor2.e >> 4) && (apx3 = this.e(aor2.c >> 4, aor2.e >> 4)) != null) {
                    apx3.a(aor2.c & 0xF, aor2.d, aor2.e & 0xF, aor2);
                }
                this.g(aor2.c, aor2.d, aor2.e);
            }
            this.a.clear();
        }
        this.C.b();
        this.C.b();
    }

    public void a(Collection collection) {
        if (this.M) {
            this.a.addAll(collection);
        } else {
            this.g.addAll(collection);
        }
    }

    public void g(sa sa2) {
        this.a(sa2, true);
    }

    public void a(sa sa2, boolean bl2) {
        int n2 = qh.c(sa2.s);
        int n3 = qh.c(sa2.u);
        int n4 = 32;
        if (bl2 && !this.b(n2 - n4, 0, n3 - n4, n2 + n4, 0, n3 + n4)) {
            return;
        }
        sa2.S = sa2.s;
        sa2.T = sa2.t;
        sa2.U = sa2.u;
        sa2.A = sa2.y;
        sa2.B = sa2.z;
        if (bl2 && sa2.ag) {
            ++sa2.aa;
            if (sa2.m != null) {
                sa2.ab();
            } else {
                sa2.h();
            }
        }
        this.C.a("chunkCheck");
        if (Double.isNaN(sa2.s) || Double.isInfinite(sa2.s)) {
            sa2.s = sa2.S;
        }
        if (Double.isNaN(sa2.t) || Double.isInfinite(sa2.t)) {
            sa2.t = sa2.T;
        }
        if (Double.isNaN(sa2.u) || Double.isInfinite(sa2.u)) {
            sa2.u = sa2.U;
        }
        if (Double.isNaN(sa2.z) || Double.isInfinite(sa2.z)) {
            sa2.z = sa2.B;
        }
        if (Double.isNaN(sa2.y) || Double.isInfinite(sa2.y)) {
            sa2.y = sa2.A;
        }
        int n5 = qh.c(sa2.s / 16.0);
        int n6 = qh.c(sa2.t / 16.0);
        int n7 = qh.c(sa2.u / 16.0);
        if (!sa2.ag || sa2.ah != n5 || sa2.ai != n6 || sa2.aj != n7) {
            if (sa2.ag && this.c(sa2.ah, sa2.aj)) {
                this.e(sa2.ah, sa2.aj).a(sa2, sa2.ai);
            }
            if (this.c(n5, n7)) {
                sa2.ag = true;
                this.e(n5, n7).a(sa2);
            } else {
                sa2.ag = false;
            }
        }
        this.C.b();
        if (bl2 && sa2.ag && sa2.l != null) {
            if (sa2.l.K || sa2.l.m != sa2) {
                sa2.l.m = null;
                sa2.l = null;
            } else {
                this.g(sa2.l);
            }
        }
    }

    public boolean b(azt azt2) {
        return this.a(azt2, (sa)null);
    }

    public boolean a(azt azt2, sa sa2) {
        List list = this.b(null, azt2);
        for (int i2 = 0; i2 < list.size(); ++i2) {
            sa sa3 = (sa)list.get(i2);
            if (sa3.K || !sa3.k || sa3 == sa2) continue;
            return false;
        }
        return true;
    }

    public boolean c(azt azt2) {
        int n2 = qh.c(azt2.a);
        int n3 = qh.c(azt2.d + 1.0);
        int n4 = qh.c(azt2.b);
        int n5 = qh.c(azt2.e + 1.0);
        int n6 = qh.c(azt2.c);
        int n7 = qh.c(azt2.f + 1.0);
        if (azt2.a < 0.0) {
            --n2;
        }
        if (azt2.b < 0.0) {
            --n4;
        }
        if (azt2.c < 0.0) {
            --n6;
        }
        for (int i2 = n2; i2 < n3; ++i2) {
            for (int i3 = n4; i3 < n5; ++i3) {
                for (int i4 = n6; i4 < n7; ++i4) {
                    aji aji2 = this.a(i2, i3, i4);
                    if (aji2.o() == awt.a) continue;
                    return true;
                }
            }
        }
        return false;
    }

    public boolean d(azt azt2) {
        int n2 = qh.c(azt2.a);
        int n3 = qh.c(azt2.d + 1.0);
        int n4 = qh.c(azt2.b);
        int n5 = qh.c(azt2.e + 1.0);
        int n6 = qh.c(azt2.c);
        int n7 = qh.c(azt2.f + 1.0);
        if (azt2.a < 0.0) {
            --n2;
        }
        if (azt2.b < 0.0) {
            --n4;
        }
        if (azt2.c < 0.0) {
            --n6;
        }
        for (int i2 = n2; i2 < n3; ++i2) {
            for (int i3 = n4; i3 < n5; ++i3) {
                for (int i4 = n6; i4 < n7; ++i4) {
                    aji aji2 = this.a(i2, i3, i4);
                    if (!aji2.o().d()) continue;
                    return true;
                }
            }
        }
        return false;
    }

    public boolean e(azt azt2) {
        int n2;
        int n3 = qh.c(azt2.a);
        int n4 = qh.c(azt2.d + 1.0);
        int n5 = qh.c(azt2.b);
        int n6 = qh.c(azt2.e + 1.0);
        int n7 = qh.c(azt2.c);
        if (this.b(n3, n5, n7, n4, n6, n2 = qh.c(azt2.f + 1.0))) {
            for (int i2 = n3; i2 < n4; ++i2) {
                for (int i3 = n5; i3 < n6; ++i3) {
                    for (int i4 = n7; i4 < n2; ++i4) {
                        aji aji2 = this.a(i2, i3, i4);
                        if (aji2 != ajn.ab && aji2 != ajn.k && aji2 != ajn.l) continue;
                        return true;
                    }
                }
            }
        }
        return false;
    }

    public boolean a(azt azt2, awt awt2, sa sa2) {
        int n2;
        int n3 = qh.c(azt2.a);
        int n4 = qh.c(azt2.d + 1.0);
        int n5 = qh.c(azt2.b);
        int n6 = qh.c(azt2.e + 1.0);
        int n7 = qh.c(azt2.c);
        if (!this.b(n3, n5, n7, n4, n6, n2 = qh.c(azt2.f + 1.0))) {
            return false;
        }
        boolean bl2 = false;
        azw azw2 = azw.a(0.0, 0.0, 0.0);
        for (int i2 = n3; i2 < n4; ++i2) {
            for (int i3 = n5; i3 < n6; ++i3) {
                for (int i4 = n7; i4 < n2; ++i4) {
                    double d2;
                    aji aji2 = this.a(i2, i3, i4);
                    if (aji2.o() != awt2 || !((double)n6 >= (d2 = (double)((float)(i3 + 1) - alw.b(this.e(i2, i3, i4)))))) continue;
                    bl2 = true;
                    aji2.a(this, i2, i3, i4, sa2, azw2);
                }
            }
        }
        if (azw2.b() > 0.0 && sa2.aC()) {
            azw2 = azw2.a();
            double d3 = 0.014;
            sa2.v += azw2.a * d3;
            sa2.w += azw2.b * d3;
            sa2.x += azw2.c * d3;
        }
        return bl2;
    }

    public boolean a(azt azt2, awt awt2) {
        int n2 = qh.c(azt2.a);
        int n3 = qh.c(azt2.d + 1.0);
        int n4 = qh.c(azt2.b);
        int n5 = qh.c(azt2.e + 1.0);
        int n6 = qh.c(azt2.c);
        int n7 = qh.c(azt2.f + 1.0);
        for (int i2 = n2; i2 < n3; ++i2) {
            for (int i3 = n4; i3 < n5; ++i3) {
                for (int i4 = n6; i4 < n7; ++i4) {
                    if (this.a(i2, i3, i4).o() != awt2) continue;
                    return true;
                }
            }
        }
        return false;
    }

    public boolean b(azt azt2, awt awt2) {
        int n2 = qh.c(azt2.a);
        int n3 = qh.c(azt2.d + 1.0);
        int n4 = qh.c(azt2.b);
        int n5 = qh.c(azt2.e + 1.0);
        int n6 = qh.c(azt2.c);
        int n7 = qh.c(azt2.f + 1.0);
        for (int i2 = n2; i2 < n3; ++i2) {
            for (int i3 = n4; i3 < n5; ++i3) {
                for (int i4 = n6; i4 < n7; ++i4) {
                    aji aji2 = this.a(i2, i3, i4);
                    if (aji2.o() != awt2) continue;
                    int n8 = this.e(i2, i3, i4);
                    double d2 = i3 + 1;
                    if (n8 < 8) {
                        d2 = (double)(i3 + 1) - (double)n8 / 8.0;
                    }
                    if (!(d2 >= azt2.b)) continue;
                    return true;
                }
            }
        }
        return false;
    }

    public agw a(sa sa2, double d2, double d3, double d4, float f2, boolean bl2) {
        return this.a(sa2, d2, d3, d4, f2, false, bl2);
    }

    public agw a(sa sa2, double d2, double d3, double d4, float f2, boolean bl2, boolean bl3) {
        agw agw2 = new agw(this, sa2, d2, d3, d4, f2);
        agw2.a = bl2;
        agw2.b = bl3;
        agw2.a();
        agw2.a(true);
        return agw2;
    }

    public float a(azw azw2, azt azt2) {
        double d2 = 1.0 / ((azt2.d - azt2.a) * 2.0 + 1.0);
        double d3 = 1.0 / ((azt2.e - azt2.b) * 2.0 + 1.0);
        double d4 = 1.0 / ((azt2.f - azt2.c) * 2.0 + 1.0);
        if (d2 < 0.0 || d3 < 0.0 || d4 < 0.0) {
            return 0.0f;
        }
        int n2 = 0;
        int n3 = 0;
        float f2 = 0.0f;
        while (f2 <= 1.0f) {
            float f3 = 0.0f;
            while (f3 <= 1.0f) {
                float f4 = 0.0f;
                while (f4 <= 1.0f) {
                    double d5 = azt2.a + (azt2.d - azt2.a) * (double)f2;
                    double d6 = azt2.b + (azt2.e - azt2.b) * (double)f3;
                    double d7 = azt2.c + (azt2.f - azt2.c) * (double)f4;
                    if (this.a(azw.a(d5, d6, d7), azw2) == null) {
                        ++n2;
                    }
                    ++n3;
                    f4 = (float)((double)f4 + d4);
                }
                f3 = (float)((double)f3 + d3);
            }
            f2 = (float)((double)f2 + d2);
        }
        return (float)n2 / (float)n3;
    }

    public boolean a(yz yz2, int n2, int n3, int n4, int n5) {
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
        if (this.a(n2, n3, n4) == ajn.ab) {
            this.a(yz2, 1004, n2, n3, n4, 0);
            this.f(n2, n3, n4);
            return true;
        }
        return false;
    }

    @Override
    public aor o(int n2, int n3, int n4) {
        apx apx2;
        aor aor2;
        int n5;
        if (n3 < 0 || n3 >= 256) {
            return null;
        }
        aor aor3 = null;
        if (this.M) {
            for (n5 = 0; n5 < this.a.size(); ++n5) {
                aor2 = (aor)this.a.get(n5);
                if (aor2.r() || aor2.c != n2 || aor2.d != n3 || aor2.e != n4) continue;
                aor3 = aor2;
                break;
            }
        }
        if (aor3 == null && (apx2 = this.e(n2 >> 4, n4 >> 4)) != null) {
            aor3 = apx2.e(n2 & 0xF, n3, n4 & 0xF);
        }
        if (aor3 == null) {
            for (n5 = 0; n5 < this.a.size(); ++n5) {
                aor2 = (aor)this.a.get(n5);
                if (aor2.r() || aor2.c != n2 || aor2.d != n3 || aor2.e != n4) continue;
                aor3 = aor2;
                break;
            }
        }
        return aor3;
    }

    public void a(int n2, int n3, int n4, aor aor2) {
        if (aor2 != null && !aor2.r()) {
            if (this.M) {
                aor2.c = n2;
                aor2.d = n3;
                aor2.e = n4;
                Iterator iterator = this.a.iterator();
                while (iterator.hasNext()) {
                    aor aor3 = (aor)iterator.next();
                    if (aor3.c != n2 || aor3.d != n3 || aor3.e != n4) continue;
                    aor3.s();
                    iterator.remove();
                }
                this.a.add(aor2);
            } else {
                this.g.add(aor2);
                apx apx2 = this.e(n2 >> 4, n4 >> 4);
                if (apx2 != null) {
                    apx2.a(n2 & 0xF, n3, n4 & 0xF, aor2);
                }
            }
        }
    }

    public void p(int n2, int n3, int n4) {
        aor aor2 = this.o(n2, n3, n4);
        if (aor2 != null && this.M) {
            aor2.s();
            this.a.remove(aor2);
        } else {
            apx apx2;
            if (aor2 != null) {
                this.a.remove(aor2);
                this.g.remove(aor2);
            }
            if ((apx2 = this.e(n2 >> 4, n4 >> 4)) != null) {
                apx2.f(n2 & 0xF, n3, n4 & 0xF);
            }
        }
    }

    public void a(aor aor2) {
        this.b.add(aor2);
    }

    public boolean q(int n2, int n3, int n4) {
        azt azt2 = this.a(n2, n3, n4).a(this, n2, n3, n4);
        return azt2 != null && azt2.a() >= 1.0;
    }

    public static boolean a(ahl ahl2, int n2, int n3, int n4) {
        aji aji2 = ahl2.a(n2, n3, n4);
        int n5 = ahl2.e(n2, n3, n4);
        if (aji2.o().k() && aji2.d()) {
            return true;
        }
        if (aji2 instanceof ans) {
            return (n5 & 4) == 4;
        }
        if (aji2 instanceof alj) {
            return (n5 & 8) == 8;
        }
        if (aji2 instanceof aln) {
            return true;
        }
        if (aji2 instanceof ann) {
            return (n5 & 7) == 7;
        }
        return false;
    }

    public boolean c(int n2, int n3, int n4, boolean bl2) {
        if (n2 < -30000000 || n4 < -30000000 || n2 >= 30000000 || n4 >= 30000000) {
            return bl2;
        }
        apx apx2 = this.v.d(n2 >> 4, n4 >> 4);
        if (apx2 == null || apx2.g()) {
            return bl2;
        }
        aji aji2 = this.a(n2, n3, n4);
        return aji2.o().k() && aji2.d();
    }

    public void B() {
        int n2 = this.a(1.0f);
        if (n2 != this.j) {
            this.j = n2;
        }
    }

    public void a(boolean bl2, boolean bl3) {
        this.G = bl2;
        this.H = bl3;
    }

    public void b() {
        this.o();
    }

    private void a() {
        if (this.x.p()) {
            this.n = 1.0f;
            if (this.x.n()) {
                this.p = 1.0f;
            }
        }
    }

    protected void o() {
        if (this.t.g) {
            return;
        }
        if (this.E) {
            return;
        }
        int n2 = this.x.o();
        if (n2 <= 0) {
            if (this.x.n()) {
                this.x.f(this.s.nextInt(12000) + 3600);
            } else {
                this.x.f(this.s.nextInt(168000) + 12000);
            }
        } else {
            this.x.f(--n2);
            if (n2 <= 0) {
                this.x.a(!this.x.n());
            }
        }
        this.o = this.p;
        this.p = this.x.n() ? (float)((double)this.p + 0.01) : (float)((double)this.p - 0.01);
        this.p = qh.a(this.p, 0.0f, 1.0f);
        int n3 = this.x.q();
        if (n3 <= 0) {
            if (this.x.p()) {
                this.x.g(this.s.nextInt(12000) + 12000);
            } else {
                this.x.g(this.s.nextInt(168000) + 12000);
            }
        } else {
            this.x.g(--n3);
            if (n3 <= 0) {
                this.x.b(!this.x.p());
            }
        }
        this.m = this.n;
        this.n = this.x.p() ? (float)((double)this.n + 0.01) : (float)((double)this.n - 0.01);
        this.n = qh.a(this.n, 0.0f, 1.0f);
    }

    protected void C() {
        int n2;
        int n3;
        int n4;
        yz yz2;
        int n5;
        this.F.clear();
        this.C.a("buildList");
        for (n5 = 0; n5 < this.h.size(); ++n5) {
            yz2 = (yz)this.h.get(n5);
            n4 = qh.c(yz2.s / 16.0);
            n3 = qh.c(yz2.u / 16.0);
            n2 = this.p();
            for (int i2 = -n2; i2 <= n2; ++i2) {
                for (int i3 = -n2; i3 <= n2; ++i3) {
                    this.F.add(new agu(i2 + n4, i3 + n3));
                }
            }
        }
        this.C.b();
        if (this.K > 0) {
            --this.K;
        }
        this.C.a("playerCheckLight");
        if (!this.h.isEmpty()) {
            n5 = this.s.nextInt(this.h.size());
            yz2 = (yz)this.h.get(n5);
            n4 = qh.c(yz2.s) + this.s.nextInt(11) - 5;
            n3 = qh.c(yz2.t) + this.s.nextInt(11) - 5;
            n2 = qh.c(yz2.u) + this.s.nextInt(11) - 5;
            this.t(n4, n3, n2);
        }
        this.C.b();
    }

    protected abstract int p();

    protected void a(int n2, int n3, apx apx2) {
        this.C.c("moodSound");
        if (this.K == 0 && !this.E) {
            yz yz2;
            this.k = this.k * 3 + 1013904223;
            int n4 = this.k >> 2;
            int n5 = n4 & 0xF;
            int n6 = n4 >> 8 & 0xF;
            int n7 = n4 >> 16 & 0xFF;
            aji aji2 = apx2.a(n5, n7, n6);
            if (aji2.o() == awt.a && this.j(n5 += n2, n7, n6 += n3) <= this.s.nextInt(8) && this.b(ahn.a, n5, n7, n6) <= 0 && (yz2 = this.a((double)n5 + 0.5, (double)n7 + 0.5, (double)n6 + 0.5, 8.0)) != null && yz2.e((double)n5 + 0.5, (double)n7 + 0.5, (double)n6 + 0.5) > 4.0) {
                this.a((double)n5 + 0.5, (double)n7 + 0.5, (double)n6 + 0.5, "ambient.cave.cave", 0.7f, 0.8f + this.s.nextFloat() * 0.2f);
                this.K = this.s.nextInt(12000) + 6000;
            }
        }
        this.C.c("checkLight");
        apx2.o();
    }

    protected void g() {
        this.C();
    }

    public boolean r(int n2, int n3, int n4) {
        return this.d(n2, n3, n4, false);
    }

    public boolean s(int n2, int n3, int n4) {
        return this.d(n2, n3, n4, true);
    }

    public boolean d(int n2, int n3, int n4, boolean bl2) {
        aji aji2;
        ahu ahu2 = this.a(n2, n4);
        float f2 = ahu2.a(n2, n3, n4);
        if (f2 > 0.15f) {
            return false;
        }
        if (n3 >= 0 && n3 < 256 && this.b(ahn.b, n2, n3, n4) < 10 && ((aji2 = this.a(n2, n3, n4)) == ajn.j || aji2 == ajn.i) && this.e(n2, n3, n4) == 0) {
            if (!bl2) {
                return true;
            }
            boolean bl3 = true;
            if (bl3 && this.a(n2 - 1, n3, n4).o() != awt.h) {
                bl3 = false;
            }
            if (bl3 && this.a(n2 + 1, n3, n4).o() != awt.h) {
                bl3 = false;
            }
            if (bl3 && this.a(n2, n3, n4 - 1).o() != awt.h) {
                bl3 = false;
            }
            if (bl3 && this.a(n2, n3, n4 + 1).o() != awt.h) {
                bl3 = false;
            }
            if (!bl3) {
                return true;
            }
        }
        return false;
    }

    public boolean e(int n2, int n3, int n4, boolean bl2) {
        aji aji2;
        ahu ahu2 = this.a(n2, n4);
        float f2 = ahu2.a(n2, n3, n4);
        if (f2 > 0.15f) {
            return false;
        }
        if (!bl2) {
            return true;
        }
        return n3 >= 0 && n3 < 256 && this.b(ahn.b, n2, n3, n4) < 10 && (aji2 = this.a(n2, n3, n4)).o() == awt.a && ajn.aC.c(this, n2, n3, n4);
    }

    public boolean t(int n2, int n3, int n4) {
        boolean bl2 = false;
        if (!this.t.g) {
            bl2 |= this.c(ahn.a, n2, n3, n4);
        }
        return bl2 |= this.c(ahn.b, n2, n3, n4);
    }

    private int a(int n2, int n3, int n4, ahn ahn2) {
        if (ahn2 == ahn.a && this.i(n2, n3, n4)) {
            return 15;
        }
        aji aji2 = this.a(n2, n3, n4);
        int n5 = ahn2 == ahn.a ? 0 : aji2.m();
        int n6 = aji2.k();
        if (n6 >= 15 && aji2.m() > 0) {
            n6 = 1;
        }
        if (n6 < 1) {
            n6 = 1;
        }
        if (n6 >= 15) {
            return 0;
        }
        if (n5 >= 14) {
            return n5;
        }
        for (int i2 = 0; i2 < 6; ++i2) {
            int n7 = n2 + q.b[i2];
            int n8 = n3 + q.c[i2];
            int n9 = n4 + q.d[i2];
            int n10 = this.b(ahn2, n7, n8, n9) - n6;
            if (n10 > n5) {
                n5 = n10;
            }
            if (n5 < 14) continue;
            return n5;
        }
        return n5;
    }

    public boolean c(ahn ahn2, int n2, int n3, int n4) {
        int n5;
        int n6;
        int n7;
        int n8;
        int n9;
        int n10;
        int n11;
        int n12;
        int n13;
        int n14;
        if (!this.a(n2, n3, n4, 17)) {
            return false;
        }
        int n15 = 0;
        int n16 = 0;
        this.C.a("getBrightness");
        int n17 = this.b(ahn2, n2, n3, n4);
        int n18 = this.a(n2, n3, n4, ahn2);
        if (n18 > n17) {
            this.I[n16++] = 133152;
        } else if (n18 < n17) {
            this.I[n16++] = 0x20820 | n17 << 18;
            while (n15 < n16) {
                n14 = this.I[n15++];
                n13 = (n14 & 0x3F) - 32 + n2;
                n12 = (n14 >> 6 & 0x3F) - 32 + n3;
                n11 = (n14 >> 12 & 0x3F) - 32 + n4;
                n10 = n14 >> 18 & 0xF;
                n9 = this.b(ahn2, n13, n12, n11);
                if (n9 != n10) continue;
                this.b(ahn2, n13, n12, n11, 0);
                if (n10 <= 0 || (n8 = qh.a(n13 - n2)) + (n7 = qh.a(n12 - n3)) + (n6 = qh.a(n11 - n4)) >= 17) continue;
                for (n5 = 0; n5 < 6; ++n5) {
                    int n19 = n13 + q.b[n5];
                    int n20 = n12 + q.c[n5];
                    int n21 = n11 + q.d[n5];
                    int n22 = Math.max(1, this.a(n19, n20, n21).k());
                    n9 = this.b(ahn2, n19, n20, n21);
                    if (n9 != n10 - n22 || n16 >= this.I.length) continue;
                    this.I[n16++] = n19 - n2 + 32 | n20 - n3 + 32 << 6 | n21 - n4 + 32 << 12 | n10 - n22 << 18;
                }
            }
            n15 = 0;
        }
        this.C.b();
        this.C.a("checkedPosition < toCheckCount");
        while (n15 < n16) {
            n14 = this.I[n15++];
            n13 = (n14 & 0x3F) - 32 + n2;
            n12 = (n14 >> 6 & 0x3F) - 32 + n3;
            n11 = (n14 >> 12 & 0x3F) - 32 + n4;
            n10 = this.b(ahn2, n13, n12, n11);
            n9 = this.a(n13, n12, n11, ahn2);
            if (n9 == n10) continue;
            this.b(ahn2, n13, n12, n11, n9);
            if (n9 <= n10) continue;
            n8 = Math.abs(n13 - n2);
            n7 = Math.abs(n12 - n3);
            n6 = Math.abs(n11 - n4);
            int n23 = n5 = n16 < this.I.length - 6 ? 1 : 0;
            if (n8 + n7 + n6 >= 17 || n5 == 0) continue;
            if (this.b(ahn2, n13 - 1, n12, n11) < n9) {
                this.I[n16++] = n13 - 1 - n2 + 32 + (n12 - n3 + 32 << 6) + (n11 - n4 + 32 << 12);
            }
            if (this.b(ahn2, n13 + 1, n12, n11) < n9) {
                this.I[n16++] = n13 + 1 - n2 + 32 + (n12 - n3 + 32 << 6) + (n11 - n4 + 32 << 12);
            }
            if (this.b(ahn2, n13, n12 - 1, n11) < n9) {
                this.I[n16++] = n13 - n2 + 32 + (n12 - 1 - n3 + 32 << 6) + (n11 - n4 + 32 << 12);
            }
            if (this.b(ahn2, n13, n12 + 1, n11) < n9) {
                this.I[n16++] = n13 - n2 + 32 + (n12 + 1 - n3 + 32 << 6) + (n11 - n4 + 32 << 12);
            }
            if (this.b(ahn2, n13, n12, n11 - 1) < n9) {
                this.I[n16++] = n13 - n2 + 32 + (n12 - n3 + 32 << 6) + (n11 - 1 - n4 + 32 << 12);
            }
            if (this.b(ahn2, n13, n12, n11 + 1) >= n9) continue;
            this.I[n16++] = n13 - n2 + 32 + (n12 - n3 + 32 << 6) + (n11 + 1 - n4 + 32 << 12);
        }
        this.C.b();
        return true;
    }

    public boolean a(boolean bl2) {
        return false;
    }

    public List a(apx apx2, boolean bl2) {
        return null;
    }

    public List b(sa sa2, azt azt2) {
        return this.a(sa2, azt2, null);
    }

    public List a(sa sa2, azt azt2, sj sj2) {
        ArrayList arrayList = new ArrayList();
        int n2 = qh.c((azt2.a - 2.0) / 16.0);
        int n3 = qh.c((azt2.d + 2.0) / 16.0);
        int n4 = qh.c((azt2.c - 2.0) / 16.0);
        int n5 = qh.c((azt2.f + 2.0) / 16.0);
        for (int i2 = n2; i2 <= n3; ++i2) {
            for (int i3 = n4; i3 <= n5; ++i3) {
                if (!this.c(i2, i3)) continue;
                this.e(i2, i3).a(sa2, azt2, arrayList, sj2);
            }
        }
        return arrayList;
    }

    public List a(Class clazz, azt azt2) {
        return this.a(clazz, azt2, (sj)null);
    }

    public List a(Class clazz, azt azt2, sj sj2) {
        int n2 = qh.c((azt2.a - 2.0) / 16.0);
        int n3 = qh.c((azt2.d + 2.0) / 16.0);
        int n4 = qh.c((azt2.c - 2.0) / 16.0);
        int n5 = qh.c((azt2.f + 2.0) / 16.0);
        ArrayList arrayList = new ArrayList();
        for (int i2 = n2; i2 <= n3; ++i2) {
            for (int i3 = n4; i3 <= n5; ++i3) {
                if (!this.c(i2, i3)) continue;
                this.e(i2, i3).a(clazz, azt2, arrayList, sj2);
            }
        }
        return arrayList;
    }

    public sa a(Class clazz, azt azt2, sa sa2) {
        List list = this.a(clazz, azt2);
        sa sa3 = null;
        double d2 = Double.MAX_VALUE;
        for (int i2 = 0; i2 < list.size(); ++i2) {
            double d3;
            sa sa4 = (sa)list.get(i2);
            if (sa4 == sa2 || (d3 = sa2.f(sa4)) > d2) continue;
            sa3 = sa4;
            d2 = d3;
        }
        return sa3;
    }

    public abstract sa a(int var1);

    public void b(int n2, int n3, int n4, aor aor2) {
        if (this.d(n2, n3, n4)) {
            this.d(n2, n4).e();
        }
    }

    public int a(Class clazz) {
        int n2 = 0;
        for (int i2 = 0; i2 < this.e.size(); ++i2) {
            sa sa2 = (sa)this.e.get(i2);
            if (sa2 instanceof sw && ((sw)sa2).bK() || !clazz.isAssignableFrom(sa2.getClass())) continue;
            ++n2;
        }
        return n2;
    }

    public void a(List list) {
        this.e.addAll(list);
        for (int i2 = 0; i2 < list.size(); ++i2) {
            this.a((sa)list.get(i2));
        }
    }

    public void b(List list) {
        this.f.addAll(list);
    }

    public boolean a(aji aji2, int n2, int n3, int n4, boolean bl2, int n5, sa sa2, add add2) {
        azt azt2;
        aji aji3 = this.a(n2, n3, n4);
        azt azt3 = azt2 = bl2 ? null : aji2.a(this, n2, n3, n4);
        if (azt2 != null && !this.a(azt2, sa2)) {
            return false;
        }
        if (aji3.o() == awt.q && aji2 == ajn.bQ) {
            return true;
        }
        return aji3.o().j() && aji2.a(this, n2, n3, n4, n5, add2);
    }

    public ayf a(sa sa2, sa sa3, float f2, boolean bl2, boolean bl3, boolean bl4, boolean bl5) {
        this.C.a("pathfind");
        int n2 = qh.c(sa2.s);
        int n3 = qh.c(sa2.t + 1.0);
        int n4 = qh.c(sa2.u);
        int n5 = (int)(f2 + 16.0f);
        int n6 = n2 - n5;
        int n7 = n3 - n5;
        int n8 = n4 - n5;
        int n9 = n2 + n5;
        int n10 = n3 + n5;
        int n11 = n4 + n5;
        ahr ahr2 = new ahr(this, n6, n7, n8, n9, n10, n11, 0);
        ayf ayf2 = new ayg(ahr2, bl2, bl3, bl4, bl5).a(sa2, sa3, f2);
        this.C.b();
        return ayf2;
    }

    public ayf a(sa sa2, int n2, int n3, int n4, float f2, boolean bl2, boolean bl3, boolean bl4, boolean bl5) {
        this.C.a("pathfind");
        int n5 = qh.c(sa2.s);
        int n6 = qh.c(sa2.t);
        int n7 = qh.c(sa2.u);
        int n8 = (int)(f2 + 8.0f);
        int n9 = n5 - n8;
        int n10 = n6 - n8;
        int n11 = n7 - n8;
        int n12 = n5 + n8;
        int n13 = n6 + n8;
        int n14 = n7 + n8;
        ahr ahr2 = new ahr(this, n9, n10, n11, n12, n13, n14, 0);
        ayf ayf2 = new ayg(ahr2, bl2, bl3, bl4, bl5).a(sa2, n2, n3, n4, f2);
        this.C.b();
        return ayf2;
    }

    @Override
    public int e(int n2, int n3, int n4, int n5) {
        return this.a(n2, n3, n4).c((ahl)this, n2, n3, n4, n5);
    }

    public int u(int n2, int n3, int n4) {
        int n5 = 0;
        if ((n5 = Math.max(n5, this.e(n2, n3 - 1, n4, 0))) >= 15) {
            return n5;
        }
        if ((n5 = Math.max(n5, this.e(n2, n3 + 1, n4, 1))) >= 15) {
            return n5;
        }
        if ((n5 = Math.max(n5, this.e(n2, n3, n4 - 1, 2))) >= 15) {
            return n5;
        }
        if ((n5 = Math.max(n5, this.e(n2, n3, n4 + 1, 3))) >= 15) {
            return n5;
        }
        if ((n5 = Math.max(n5, this.e(n2 - 1, n3, n4, 4))) >= 15) {
            return n5;
        }
        if ((n5 = Math.max(n5, this.e(n2 + 1, n3, n4, 5))) >= 15) {
            return n5;
        }
        return n5;
    }

    public boolean f(int n2, int n3, int n4, int n5) {
        return this.g(n2, n3, n4, n5) > 0;
    }

    public int g(int n2, int n3, int n4, int n5) {
        if (this.a(n2, n3, n4).r()) {
            return this.u(n2, n3, n4);
        }
        return this.a(n2, n3, n4).b((ahl)this, n2, n3, n4, n5);
    }

    public boolean v(int n2, int n3, int n4) {
        if (this.g(n2, n3 - 1, n4, 0) > 0) {
            return true;
        }
        if (this.g(n2, n3 + 1, n4, 1) > 0) {
            return true;
        }
        if (this.g(n2, n3, n4 - 1, 2) > 0) {
            return true;
        }
        if (this.g(n2, n3, n4 + 1, 3) > 0) {
            return true;
        }
        if (this.g(n2 - 1, n3, n4, 4) > 0) {
            return true;
        }
        return this.g(n2 + 1, n3, n4, 5) > 0;
    }

    public int w(int n2, int n3, int n4) {
        int n5 = 0;
        for (int i2 = 0; i2 < 6; ++i2) {
            int n6 = this.g(n2 + q.b[i2], n3 + q.c[i2], n4 + q.d[i2], i2);
            if (n6 >= 15) {
                return 15;
            }
            if (n6 <= n5) continue;
            n5 = n6;
        }
        return n5;
    }

    public yz a(sa sa2, double d2) {
        return this.a(sa2.s, sa2.t, sa2.u, d2);
    }

    public yz a(double d2, double d3, double d4, double d5) {
        double d6 = -1.0;
        yz yz2 = null;
        for (int i2 = 0; i2 < this.h.size(); ++i2) {
            yz yz3 = (yz)this.h.get(i2);
            double d7 = yz3.e(d2, d3, d4);
            if (!(d5 < 0.0) && !(d7 < d5 * d5) || d6 != -1.0 && !(d7 < d6)) continue;
            d6 = d7;
            yz2 = yz3;
        }
        return yz2;
    }

    public yz b(sa sa2, double d2) {
        return this.b(sa2.s, sa2.t, sa2.u, d2);
    }

    public yz b(double d2, double d3, double d4, double d5) {
        double d6 = -1.0;
        yz yz2 = null;
        for (int i2 = 0; i2 < this.h.size(); ++i2) {
            yz yz3 = (yz)this.h.get(i2);
            if (yz3.bE.a || !yz3.Z()) continue;
            double d7 = yz3.e(d2, d3, d4);
            double d8 = d5;
            if (yz3.an()) {
                d8 *= (double)0.8f;
            }
            if (yz3.ap()) {
                float f2 = yz3.bE();
                if (f2 < 0.1f) {
                    f2 = 0.1f;
                }
                d8 *= (double)(0.7f * f2);
            }
            if (!(d5 < 0.0) && !(d7 < d8 * d8) || d6 != -1.0 && !(d7 < d6)) continue;
            d6 = d7;
            yz2 = yz3;
        }
        return yz2;
    }

    public yz a(String string) {
        for (int i2 = 0; i2 < this.h.size(); ++i2) {
            yz yz2 = (yz)this.h.get(i2);
            if (!string.equals(yz2.b_())) continue;
            return yz2;
        }
        return null;
    }

    public yz a(UUID uUID) {
        for (int i2 = 0; i2 < this.h.size(); ++i2) {
            yz yz2 = (yz)this.h.get(i2);
            if (!uUID.equals(yz2.aB())) continue;
            return yz2;
        }
        return null;
    }

    public void G() {
        this.w.c();
    }

    public long H() {
        return this.x.b();
    }

    public long I() {
        return this.x.f();
    }

    public long J() {
        return this.x.g();
    }

    public void b(long l2) {
        this.x.c(l2);
    }

    public r K() {
        return new r(this.x.c(), this.x.d(), this.x.e());
    }

    public void x(int n2, int n3, int n4) {
        this.x.a(n2, n3, n4);
    }

    public boolean a(yz yz2, int n2, int n3, int n4) {
        return true;
    }

    public void a(sa sa2, byte by2) {
    }

    public apu L() {
        return this.v;
    }

    public void c(int n2, int n3, int n4, aji aji2, int n5, int n6) {
        aji2.a(this, n2, n3, n4, n5, n6);
    }

    public azc M() {
        return this.w;
    }

    public ays N() {
        return this.x;
    }

    public agy O() {
        return this.x.x();
    }

    public void c() {
    }

    public float h(float f2) {
        return (this.o + (this.p - this.o) * f2) * this.j(f2);
    }

    public float j(float f2) {
        return this.m + (this.n - this.m) * f2;
    }

    public boolean P() {
        return (double)this.h(1.0f) > 0.9;
    }

    public boolean Q() {
        return (double)this.j(1.0f) > 0.2;
    }

    public boolean y(int n2, int n3, int n4) {
        if (!this.Q()) {
            return false;
        }
        if (!this.i(n2, n3, n4)) {
            return false;
        }
        if (this.h(n2, n4) > n3) {
            return false;
        }
        ahu ahu2 = this.a(n2, n4);
        if (ahu2.d()) {
            return false;
        }
        if (this.e(n2, n3, n4, false)) {
            return false;
        }
        return ahu2.e();
    }

    public boolean z(int n2, int n3, int n4) {
        ahu ahu2 = this.a(n2, n4);
        return ahu2.f();
    }

    public void a(String string, ayl ayl2) {
        this.z.a(string, ayl2);
    }

    public ayl a(Class clazz, String string) {
        return this.z.a(clazz, string);
    }

    public int b(String string) {
        return this.z.a(string);
    }

    public void b(int n2, int n3, int n4, int n5, int n6) {
        for (int i2 = 0; i2 < this.u.size(); ++i2) {
            ((ahh)this.u.get(i2)).a(n2, n3, n4, n5, n6);
        }
    }

    public void c(int n2, int n3, int n4, int n5, int n6) {
        this.a(null, n2, n3, n4, n5, n6);
    }

    public void a(yz yz2, int n2, int n3, int n4, int n5, int n6) {
        try {
            for (int i2 = 0; i2 < this.u.size(); ++i2) {
                ((ahh)this.u.get(i2)).a(yz2, n2, n3, n4, n5, n6);
            }
        }
        catch (Throwable throwable) {
            b b2 = b.a(throwable, "Playing level event");
            k k2 = b2.a("Level event being played");
            k2.a("Block coordinates", k.a(n3, n4, n5));
            k2.a("Event source", yz2);
            k2.a("Event type", n2);
            k2.a("Event data", n6);
            throw new s(b2);
        }
    }

    public int R() {
        return 256;
    }

    public int S() {
        return this.t.g ? 128 : 256;
    }

    public Random A(int n2, int n3, int n4) {
        long l2 = (long)n2 * 341873128712L + (long)n3 * 132897987541L + this.N().b() + (long)n4;
        this.s.setSeed(l2);
        return this.s;
    }

    public agt b(String string, int n2, int n3, int n4) {
        return this.L().a(this, string, n2, n3, n4);
    }

    public k a(b b2) {
        k k2 = b2.a("Affected level", 1);
        k2.a("Level name", this.x == null ? "????" : this.x.k());
        k2.a("All players", new ahe(this));
        k2.a("Chunk stats", new ahf(this));
        try {
            this.x.a(k2);
        }
        catch (Throwable throwable) {
            k2.a("Level Data Unobtainable", throwable);
        }
        return k2;
    }

    public void d(int n2, int n3, int n4, int n5, int n6) {
        for (int i2 = 0; i2 < this.u.size(); ++i2) {
            ahh ahh2 = (ahh)this.u.get(i2);
            ahh2.b(n2, n3, n4, n5, n6);
        }
    }

    public Calendar V() {
        if (this.I() % 600L == 0L) {
            this.J.setTimeInMillis(MinecraftServer.ar());
        }
        return this.J;
    }

    public bac W() {
        return this.D;
    }

    public void f(int n2, int n3, int n4, aji aji2) {
        for (int i2 = 0; i2 < 4; ++i2) {
            aji aji3;
            int n5 = n2 + p.a[i2];
            int n6 = n4 + p.b[i2];
            aji aji4 = this.a(n5, n3, n6);
            if (ajn.bU.e(aji4)) {
                aji4.a(this, n5, n3, n6, aji2);
                continue;
            }
            if (!aji4.r() || !ajn.bU.e(aji3 = this.a(n5 += p.a[i2], n3, n6 += p.b[i2]))) continue;
            aji3.a(this, n5, n3, n6, aji2);
        }
    }

    public float b(double d2, double d3, double d4) {
        return this.B(qh.c(d2), qh.c(d3), qh.c(d4));
    }

    public float B(int n2, int n3, int n4) {
        boolean bl2;
        float f2 = 0.0f;
        boolean bl3 = bl2 = this.r == rd.d;
        if (this.d(n2, n3, n4)) {
            float f3 = this.y();
            f2 += qh.a((float)this.d((int)n2, (int)n4).s / 3600000.0f, 0.0f, 1.0f) * (bl2 ? 1.0f : 0.75f);
            f2 += f3 * 0.25f;
        }
        if (this.r == rd.b || this.r == rd.a) {
            f2 *= (float)this.r.a() / 2.0f;
        }
        return qh.a(f2, 0.0f, bl2 ? 1.5f : 1.0f);
    }

    public void X() {
        for (ahh ahh2 : this.u) {
            ahh2.b();
        }
    }
}

