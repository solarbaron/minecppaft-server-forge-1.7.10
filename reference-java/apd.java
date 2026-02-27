/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class apd
extends aor {
    public int a;
    public float i;
    public float j;
    public float k;
    public float l;
    public float m;
    public float n;
    public float o;
    public float p;
    public float q;
    private static Random r = new Random();
    private String s;

    @Override
    public void b(dh dh2) {
        super.b(dh2);
        if (this.b()) {
            dh2.a("CustomName", this.s);
        }
    }

    @Override
    public void a(dh dh2) {
        super.a(dh2);
        if (dh2.b("CustomName", 8)) {
            this.s = dh2.j("CustomName");
        }
    }

    @Override
    public void h() {
        float f2;
        super.h();
        this.n = this.m;
        this.p = this.o;
        yz yz2 = this.b.a((float)this.c + 0.5f, (float)this.d + 0.5f, (double)((float)this.e + 0.5f), 3.0);
        if (yz2 != null) {
            double d2 = yz2.s - (double)((float)this.c + 0.5f);
            double d3 = yz2.u - (double)((float)this.e + 0.5f);
            this.q = (float)Math.atan2(d3, d2);
            this.m += 0.1f;
            if (this.m < 0.5f || r.nextInt(40) == 0) {
                float f3 = this.k;
                do {
                    this.k += (float)(r.nextInt(4) - r.nextInt(4));
                } while (f3 == this.k);
            }
        } else {
            this.q += 0.02f;
            this.m -= 0.1f;
        }
        while (this.o >= (float)Math.PI) {
            this.o -= (float)Math.PI * 2;
        }
        while (this.o < (float)(-Math.PI)) {
            this.o += (float)Math.PI * 2;
        }
        while (this.q >= (float)Math.PI) {
            this.q -= (float)Math.PI * 2;
        }
        while (this.q < (float)(-Math.PI)) {
            this.q += (float)Math.PI * 2;
        }
        for (f2 = this.q - this.o; f2 >= (float)Math.PI; f2 -= (float)Math.PI * 2) {
        }
        while (f2 < (float)(-Math.PI)) {
            f2 += (float)Math.PI * 2;
        }
        this.o += f2 * 0.4f;
        if (this.m < 0.0f) {
            this.m = 0.0f;
        }
        if (this.m > 1.0f) {
            this.m = 1.0f;
        }
        ++this.a;
        this.j = this.i;
        float f4 = (this.k - this.i) * 0.4f;
        float f5 = 0.2f;
        if (f4 < -f5) {
            f4 = -f5;
        }
        if (f4 > f5) {
            f4 = f5;
        }
        this.l += (f4 - this.l) * 0.9f;
        this.i += this.l;
    }

    public String a() {
        return this.b() ? this.s : "container.enchant";
    }

    public boolean b() {
        return this.s != null && this.s.length() > 0;
    }

    public void a(String string) {
        this.s = string;
    }
}

