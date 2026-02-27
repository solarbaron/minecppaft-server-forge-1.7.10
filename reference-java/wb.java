/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;

public class wb {
    private ahb a;
    private boolean b;
    private int c = -1;
    private int d;
    private int e;
    private vz f;
    private int g;
    private int h;
    private int i;

    public wb(ahb ahb2) {
        this.a = ahb2;
    }

    public void a() {
        boolean bl2 = false;
        if (bl2) {
            if (this.c == 2) {
                this.d = 100;
                return;
            }
        } else {
            if (this.a.w()) {
                this.c = 0;
                return;
            }
            if (this.c == 2) {
                return;
            }
            if (this.c == 0) {
                float f2 = this.a.c(0.0f);
                if ((double)f2 < 0.5 || (double)f2 > 0.501) {
                    return;
                }
                this.c = this.a.s.nextInt(10) == 0 ? 1 : 2;
                this.b = false;
                if (this.c == 2) {
                    return;
                }
            }
        }
        if (!this.b) {
            if (this.b()) {
                this.b = true;
            } else {
                return;
            }
        }
        if (this.e > 0) {
            --this.e;
            return;
        }
        this.e = 2;
        if (this.d > 0) {
            this.c();
            --this.d;
        } else {
            this.c = 2;
        }
    }

    private boolean b() {
        List list = this.a.h;
        for (yz yz2 : list) {
            this.f = this.a.A.a((int)yz2.s, (int)yz2.t, (int)yz2.u, 1);
            if (this.f == null || this.f.c() < 10 || this.f.d() < 20 || this.f.e() < 20) continue;
            r r2 = this.f.a();
            float f2 = this.f.b();
            boolean bl2 = false;
            for (int i2 = 0; i2 < 10; ++i2) {
                this.g = r2.a + (int)((double)(qh.b(this.a.s.nextFloat() * (float)Math.PI * 2.0f) * f2) * 0.9);
                this.h = r2.b;
                this.i = r2.c + (int)((double)(qh.a(this.a.s.nextFloat() * (float)Math.PI * 2.0f) * f2) * 0.9);
                bl2 = false;
                for (vz vz2 : this.a.A.b()) {
                    if (vz2 == this.f || !vz2.a(this.g, this.h, this.i)) continue;
                    bl2 = true;
                    break;
                }
                if (!bl2) break;
            }
            if (bl2) {
                return false;
            }
            azw azw2 = this.a(this.g, this.h, this.i);
            if (azw2 == null) continue;
            this.e = 0;
            this.d = 20;
            return true;
        }
        return false;
    }

    private boolean c() {
        yq yq2;
        azw azw2 = this.a(this.g, this.h, this.i);
        if (azw2 == null) {
            return false;
        }
        try {
            yq2 = new yq(this.a);
            yq2.a((sy)null);
            yq2.j(false);
        }
        catch (Exception exception) {
            exception.printStackTrace();
            return false;
        }
        yq2.b(azw2.a, azw2.b, azw2.c, this.a.s.nextFloat() * 360.0f, 0.0f);
        this.a.d(yq2);
        r r2 = this.f.a();
        yq2.a(r2.a, r2.b, r2.c, this.f.b());
        return true;
    }

    private azw a(int n2, int n3, int n4) {
        for (int i2 = 0; i2 < 10; ++i2) {
            int n5;
            int n6;
            int n7 = n2 + this.a.s.nextInt(16) - 8;
            if (!this.f.a(n7, n6 = n3 + this.a.s.nextInt(6) - 3, n5 = n4 + this.a.s.nextInt(16) - 8) || !aho.a(sx.a, this.a, n7, n6, n5)) continue;
            azw.a(n7, n6, n5);
        }
        return null;
    }
}

