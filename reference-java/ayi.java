/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

public class ayi
extends ayl {
    public int a;
    public int b;
    public byte c;
    public byte d;
    public byte[] e = new byte[16384];
    public List f = new ArrayList();
    private Map i = new HashMap();
    public Map g = new LinkedHashMap();

    public ayi(String string) {
        super(string);
    }

    @Override
    public void a(dh dh2) {
        this.c = dh2.d("dimension");
        this.a = dh2.f("xCenter");
        this.b = dh2.f("zCenter");
        this.d = dh2.d("scale");
        if (this.d < 0) {
            this.d = 0;
        }
        if (this.d > 4) {
            this.d = (byte)4;
        }
        int n2 = dh2.e("width");
        int n3 = dh2.e("height");
        if (n2 == 128 && n3 == 128) {
            this.e = dh2.k("colors");
        } else {
            byte[] byArray = dh2.k("colors");
            this.e = new byte[16384];
            int n4 = (128 - n2) / 2;
            int n5 = (128 - n3) / 2;
            for (int i2 = 0; i2 < n3; ++i2) {
                int n6 = i2 + n5;
                if (n6 < 0 && n6 >= 128) continue;
                for (int i3 = 0; i3 < n2; ++i3) {
                    int n7 = i3 + n4;
                    if (n7 < 0 && n7 >= 128) continue;
                    this.e[n7 + n6 * 128] = byArray[i3 + i2 * n2];
                }
            }
        }
    }

    @Override
    public void b(dh dh2) {
        dh2.a("dimension", this.c);
        dh2.a("xCenter", this.a);
        dh2.a("zCenter", this.b);
        dh2.a("scale", this.d);
        dh2.a("width", (short)128);
        dh2.a("height", (short)128);
        dh2.a("colors", this.e);
    }

    public void a(yz yz2, add add2) {
        if (!this.i.containsKey(yz2)) {
            ayj ayj2 = new ayj(this, yz2);
            this.i.put(yz2, ayj2);
            this.f.add(ayj2);
        }
        if (!yz2.bm.c(add2)) {
            this.g.remove(yz2.b_());
        }
        for (int i2 = 0; i2 < this.f.size(); ++i2) {
            ayj ayj3 = (ayj)this.f.get(i2);
            if (ayj3.a.K || !ayj3.a.bm.c(add2) && !add2.A()) {
                this.i.remove(ayj3.a);
                this.f.remove(ayj3);
                continue;
            }
            if (add2.A() || ayj3.a.ap != this.c) continue;
            this.a(0, ayj3.a.o, ayj3.a.b_(), ayj3.a.s, ayj3.a.u, ayj3.a.y);
        }
        if (add2.A()) {
            this.a(1, yz2.o, "frame-" + add2.B().y(), add2.B().b, add2.B().d, add2.B().a * 90);
        }
    }

    private void a(int n2, ahb ahb2, String string, double d2, double d3, double d4) {
        byte by2;
        int n3 = 1 << this.d;
        float f2 = (float)(d2 - (double)this.a) / (float)n3;
        float f3 = (float)(d3 - (double)this.b) / (float)n3;
        byte by3 = (byte)((double)(f2 * 2.0f) + 0.5);
        byte by4 = (byte)((double)(f3 * 2.0f) + 0.5);
        int n4 = 63;
        if (f2 >= (float)(-n4) && f3 >= (float)(-n4) && f2 <= (float)n4 && f3 <= (float)n4) {
            by2 = (byte)((d4 += d4 < 0.0 ? -8.0 : 8.0) * 16.0 / 360.0);
            if (this.c < 0) {
                int n5 = (int)(ahb2.N().g() / 10L);
                by2 = (byte)(n5 * n5 * 34187121 + n5 * 121 >> 15 & 0xF);
            }
        } else if (Math.abs(f2) < 320.0f && Math.abs(f3) < 320.0f) {
            n2 = 6;
            by2 = 0;
            if (f2 <= (float)(-n4)) {
                by3 = (byte)((double)(n4 * 2) + 2.5);
            }
            if (f3 <= (float)(-n4)) {
                by4 = (byte)((double)(n4 * 2) + 2.5);
            }
            if (f2 >= (float)n4) {
                by3 = (byte)(n4 * 2 + 1);
            }
            if (f3 >= (float)n4) {
                by4 = (byte)(n4 * 2 + 1);
            }
        } else {
            this.g.remove(string);
            return;
        }
        this.g.put(string, new ayk(this, (byte)n2, by3, by4, by2));
    }

    public byte[] a(add add2, ahb ahb2, yz yz2) {
        ayj ayj2 = (ayj)this.i.get(yz2);
        if (ayj2 == null) {
            return null;
        }
        return ayj2.a(add2);
    }

    public void a(int n2, int n3, int n4) {
        super.c();
        for (int i2 = 0; i2 < this.f.size(); ++i2) {
            ayj ayj2 = (ayj)this.f.get(i2);
            if (ayj2.b[n2] < 0 || ayj2.b[n2] > n3) {
                ayj2.b[n2] = n3;
            }
            if (ayj2.c[n2] >= 0 && ayj2.c[n2] >= n4) continue;
            ayj2.c[n2] = n4;
        }
    }

    public ayj a(yz yz2) {
        ayj ayj2 = (ayj)this.i.get(yz2);
        if (ayj2 == null) {
            ayj2 = new ayj(this, yz2);
            this.i.put(yz2, ayj2);
            this.f.add(ayj2);
        }
        return ayj2;
    }
}

