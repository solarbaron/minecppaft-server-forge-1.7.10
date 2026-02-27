/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Random;

public class aug
extends ave {
    private List e;
    private boolean f;
    private agu[] g = new agu[3];
    private double h = 32.0;
    private int i = 3;

    public aug() {
        this.e = new ArrayList();
        for (ahu ahu2 : ahu.n()) {
            if (ahu2 == null || !(ahu2.am > 0.0f)) continue;
            this.e.add(ahu2);
        }
    }

    public aug(Map map) {
        this();
        for (Map.Entry entry : map.entrySet()) {
            if (((String)entry.getKey()).equals("distance")) {
                this.h = qh.a((String)entry.getValue(), this.h, 1.0);
                continue;
            }
            if (((String)entry.getKey()).equals("count")) {
                this.g = new agu[qh.a((String)entry.getValue(), this.g.length, 1)];
                continue;
            }
            if (!((String)entry.getKey()).equals("spread")) continue;
            this.i = qh.a((String)entry.getValue(), this.i, 1);
        }
    }

    @Override
    public String a() {
        return "Stronghold";
    }

    @Override
    protected boolean a(int n2, int n3) {
        if (!this.f) {
            Random random = new Random();
            random.setSeed(this.c.H());
            double d2 = random.nextDouble() * Math.PI * 2.0;
            int n4 = 1;
            for (int i2 = 0; i2 < this.g.length; ++i2) {
                double d3 = (1.25 * (double)n4 + random.nextDouble()) * (this.h * (double)n4);
                int n5 = (int)Math.round(Math.cos(d2) * d3);
                int n6 = (int)Math.round(Math.sin(d2) * d3);
                agt agt2 = this.c.v().a((n5 << 4) + 8, (n6 << 4) + 8, 112, this.e, random);
                if (agt2 != null) {
                    n5 = agt2.a >> 4;
                    n6 = agt2.c >> 4;
                }
                this.g[i2] = new agu(n5, n6);
                d2 += Math.PI * 2 * (double)n4 / (double)this.i;
                if (i2 != this.i) continue;
                n4 += 2 + random.nextInt(5);
                this.i += 1 + random.nextInt(2);
            }
            this.f = true;
        }
        for (agu agu2 : this.g) {
            if (n2 != agu2.a || n3 != agu2.b) continue;
            return true;
        }
        return false;
    }

    @Override
    protected List o_() {
        ArrayList<agt> arrayList = new ArrayList<agt>();
        for (agu agu2 : this.g) {
            if (agu2 == null) continue;
            arrayList.add(agu2.a(64));
        }
        return arrayList;
    }

    @Override
    protected avm b(int n2, int n3) {
        auh auh2 = new auh(this.c, this.b, n2, n3);
        while (auh2.b().isEmpty() || ((auz)auh2.b().get((int)0)).b == null) {
            auh2 = new auh(this.c, this.b, n2, n3);
        }
        return auh2;
    }
}

