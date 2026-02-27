/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class xf
extends qw {
    private final add b;
    private float c;
    private boolean d;

    public xf(add add2, int n2) {
        super(n2);
        this.b = add2;
    }

    public add a(Random random) {
        add add2 = this.b.m();
        if (this.c > 0.0f) {
            int n2 = (int)(this.c * (float)this.b.l());
            int n3 = add2.l() - random.nextInt(random.nextInt(n2) + 1);
            if (n3 > n2) {
                n3 = n2;
            }
            if (n3 < 1) {
                n3 = 1;
            }
            add2.b(n3);
        }
        if (this.d) {
            afv.a(random, add2, 30);
        }
        return add2;
    }

    public xf a(float f2) {
        this.c = f2;
        return this;
    }

    public xf a() {
        this.d = true;
        return this;
    }
}

