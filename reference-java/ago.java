/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;

public class ago
extends ArrayList {
    public ago() {
    }

    public ago(dh dh2) {
        this.a(dh2);
    }

    public agn a(add add2, add add3, int n2) {
        if (n2 > 0 && n2 < this.size()) {
            agn agn2 = (agn)this.get(n2);
            if (add2.b() == agn2.a().b() && (add3 == null && !agn2.c() || agn2.c() && add3 != null && agn2.b().b() == add3.b()) && add2.b >= agn2.a().b && (!agn2.c() || add3.b >= agn2.b().b)) {
                return agn2;
            }
            return null;
        }
        for (int i2 = 0; i2 < this.size(); ++i2) {
            agn agn3 = (agn)this.get(i2);
            if (add2.b() != agn3.a().b() || add2.b < agn3.a().b || (agn3.c() || add3 != null) && (!agn3.c() || add3 == null || agn3.b().b() != add3.b() || add3.b < agn3.b().b)) continue;
            return agn3;
        }
        return null;
    }

    public void a(agn agn2) {
        for (int i2 = 0; i2 < this.size(); ++i2) {
            agn agn3 = (agn)this.get(i2);
            if (!agn2.a(agn3)) continue;
            if (agn2.b(agn3)) {
                this.set(i2, agn2);
            }
            return;
        }
        this.add(agn2);
    }

    public void a(et et2) {
        et2.writeByte((byte)(this.size() & 0xFF));
        for (int i2 = 0; i2 < this.size(); ++i2) {
            agn agn2 = (agn)this.get(i2);
            et2.a(agn2.a());
            et2.a(agn2.d());
            add add2 = agn2.b();
            et2.writeBoolean(add2 != null);
            if (add2 != null) {
                et2.a(add2);
            }
            et2.writeBoolean(agn2.g());
        }
    }

    public void a(dh dh2) {
        dq dq2 = dh2.c("Recipes", 10);
        for (int i2 = 0; i2 < dq2.c(); ++i2) {
            dh dh3 = dq2.b(i2);
            this.add(new agn(dh3));
        }
    }

    public dh a() {
        dh dh2 = new dh();
        dq dq2 = new dq();
        for (int i2 = 0; i2 < this.size(); ++i2) {
            agn agn2 = (agn)this.get(i2);
            dq2.a(agn2.i());
        }
        dh2.a("Recipes", dq2);
        return dh2;
    }
}

