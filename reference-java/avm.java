/*
 * Decompiled with CFR 0.152.
 */
import java.util.Iterator;
import java.util.LinkedList;
import java.util.Random;

public abstract class avm {
    protected LinkedList a = new LinkedList();
    protected asv b;
    private int c;
    private int d;

    public avm() {
    }

    public avm(int n2, int n3) {
        this.c = n2;
        this.d = n3;
    }

    public asv a() {
        return this.b;
    }

    public LinkedList b() {
        return this.a;
    }

    public void a(ahb ahb2, Random random, asv asv2) {
        Iterator iterator = this.a.iterator();
        while (iterator.hasNext()) {
            avk avk2 = (avk)iterator.next();
            if (!avk2.c().a(asv2) || avk2.a(ahb2, random, asv2)) continue;
            iterator.remove();
        }
    }

    protected void c() {
        this.b = asv.a();
        for (avk avk2 : this.a) {
            this.b.b(avk2.c());
        }
    }

    public dh a(int n2, int n3) {
        dh dh2 = new dh();
        dh2.a("id", avi.a(this));
        dh2.a("ChunkX", n2);
        dh2.a("ChunkZ", n3);
        dh2.a("BB", this.b.h());
        dq dq2 = new dq();
        for (avk avk2 : this.a) {
            dq2.a(avk2.b());
        }
        dh2.a("Children", dq2);
        this.a(dh2);
        return dh2;
    }

    public void a(dh dh2) {
    }

    public void a(ahb ahb2, dh dh2) {
        this.c = dh2.f("ChunkX");
        this.d = dh2.f("ChunkZ");
        if (dh2.c("BB")) {
            this.b = new asv(dh2.l("BB"));
        }
        dq dq2 = dh2.c("Children", 10);
        for (int i2 = 0; i2 < dq2.c(); ++i2) {
            this.a.add(avi.b(dq2.b(i2), ahb2));
        }
        this.b(dh2);
    }

    public void b(dh dh2) {
    }

    protected void a(ahb ahb2, Random random, int n2) {
        int n3 = 63 - n2;
        int n4 = this.b.c() + 1;
        if (n4 < n3) {
            n4 += random.nextInt(n3 - n4);
        }
        int n5 = n4 - this.b.e;
        this.b.a(0, n5, 0);
        for (avk avk2 : this.a) {
            avk2.c().a(0, n5, 0);
        }
    }

    protected void a(ahb ahb2, Random random, int n2, int n3) {
        int n4 = n3 - n2 + 1 - this.b.c();
        int n5 = 1;
        n5 = n4 > 1 ? n2 + random.nextInt(n4) : n2;
        int n6 = n5 - this.b.b;
        this.b.a(0, n6, 0);
        for (avk avk2 : this.a) {
            avk2.c().a(0, n6, 0);
        }
    }

    public boolean d() {
        return true;
    }

    public int e() {
        return this.c;
    }

    public int f() {
        return this.d;
    }
}

