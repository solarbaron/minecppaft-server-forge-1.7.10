/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class acm
extends adb {
    @Override
    public boolean e_(add add2) {
        return false;
    }

    @Override
    public adq f(add add2) {
        if (this.g(add2).c() > 0) {
            return adq.b;
        }
        return super.f(add2);
    }

    public dq g(add add2) {
        if (add2.d == null || !add2.d.b("StoredEnchantments", 9)) {
            return new dq();
        }
        return (dq)add2.d.a("StoredEnchantments");
    }

    public void a(add add2, agc agc2) {
        dq dq2 = this.g(add2);
        boolean bl2 = true;
        for (int i2 = 0; i2 < dq2.c(); ++i2) {
            dh dh2 = dq2.b(i2);
            if (dh2.e("id") != agc2.b.B) continue;
            if (dh2.e("lvl") < agc2.c) {
                dh2.a("lvl", (short)agc2.c);
            }
            bl2 = false;
            break;
        }
        if (bl2) {
            dh dh3 = new dh();
            dh3.a("id", (short)agc2.b.B);
            dh3.a("lvl", (short)agc2.c);
            dq2.a(dh3);
        }
        if (!add2.p()) {
            add2.d(new dh());
        }
        add2.q().a("StoredEnchantments", dq2);
    }

    public add a(agc agc2) {
        add add2 = new add(this);
        this.a(add2, agc2);
        return add2;
    }

    public qx b(Random random) {
        return this.a(random, 1, 1, 1);
    }

    public qx a(Random random, int n2, int n3, int n4) {
        add add2 = new add(ade.aG, 1, 0);
        afv.a(random, add2, 30);
        return new qx(add2, n2, n3, n4);
    }
}

