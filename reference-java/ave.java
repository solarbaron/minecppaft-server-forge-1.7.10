/*
 * Decompiled with CFR 0.152.
 */
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Random;

public abstract class ave
extends aqx {
    private avj e;
    protected Map d = new HashMap();

    public abstract String a();

    @Override
    protected final void a(ahb ahb2, int n2, int n3, int n4, int n5, aji[] ajiArray) {
        this.a(ahb2);
        if (this.d.containsKey(agu.a(n2, n3))) {
            return;
        }
        this.b.nextInt();
        try {
            if (this.a(n2, n3)) {
                avm avm2 = this.b(n2, n3);
                this.d.put(agu.a(n2, n3), avm2);
                this.a(n2, n3, avm2);
            }
        }
        catch (Throwable throwable) {
            b b2 = b.a(throwable, "Exception preparing structure feature");
            k k2 = b2.a("Feature being prepared");
            k2.a("Is feature chunk", new avf(this, n2, n3));
            k2.a("Chunk location", String.format("%d,%d", n2, n3));
            k2.a("Chunk pos hash", new avg(this, n2, n3));
            k2.a("Structure type", new avh(this));
            throw new s(b2);
        }
    }

    public boolean a(ahb ahb2, Random random, int n2, int n3) {
        this.a(ahb2);
        int n4 = (n2 << 4) + 8;
        int n5 = (n3 << 4) + 8;
        boolean bl2 = false;
        for (avm avm2 : this.d.values()) {
            if (!avm2.d() || !avm2.a().a(n4, n5, n4 + 15, n5 + 15)) continue;
            avm2.a(ahb2, random, new asv(n4, n5, n4 + 15, n5 + 15));
            bl2 = true;
            this.a(avm2.e(), avm2.f(), avm2);
        }
        return bl2;
    }

    public boolean b(int n2, int n3, int n4) {
        this.a(this.c);
        return this.c(n2, n3, n4) != null;
    }

    protected avm c(int n2, int n3, int n4) {
        for (avm avm2 : this.d.values()) {
            if (!avm2.d() || !avm2.a().a(n2, n4, n2, n4)) continue;
            for (avk avk2 : avm2.b()) {
                if (!avk2.c().b(n2, n3, n4)) continue;
                return avm2;
            }
        }
        return null;
    }

    public boolean d(int n2, int n3, int n4) {
        this.a(this.c);
        for (avm avm2 : this.d.values()) {
            if (!avm2.d()) continue;
            return avm2.a().a(n2, n4, n2, n4);
        }
        return false;
    }

    public agt a(ahb ahb2, int n2, int n3, int n4) {
        double d2;
        int n5;
        int n6;
        int n7;
        agt agt2;
        Object object;
        this.c = ahb2;
        this.a(ahb2);
        this.b.setSeed(ahb2.H());
        long l2 = this.b.nextLong();
        long l3 = this.b.nextLong();
        long l4 = (long)(n2 >> 4) * l2;
        long l5 = (long)(n4 >> 4) * l3;
        this.b.setSeed(l4 ^ l5 ^ ahb2.H());
        this.a(ahb2, n2 >> 4, n4 >> 4, 0, 0, null);
        double d3 = Double.MAX_VALUE;
        agt agt3 = null;
        for (Object object2 : this.d.values()) {
            if (!((avm)object2).d()) continue;
            object = (avk)((avm)object2).b().get(0);
            agt2 = ((avk)object).a();
            n7 = agt2.a - n2;
            n6 = agt2.b - n3;
            n5 = agt2.c - n4;
            d2 = n7 * n7 + n6 * n6 + n5 * n5;
            if (!(d2 < d3)) continue;
            d3 = d2;
            agt3 = agt2;
        }
        if (agt3 != null) {
            return agt3;
        }
        List list = this.o_();
        if (list != null) {
            Object object2;
            object2 = null;
            object = list.iterator();
            while (object.hasNext()) {
                agt2 = (agt)object.next();
                n7 = agt2.a - n2;
                n6 = agt2.b - n3;
                n5 = agt2.c - n4;
                d2 = n7 * n7 + n6 * n6 + n5 * n5;
                if (!(d2 < d3)) continue;
                d3 = d2;
                object2 = agt2;
            }
            return object2;
        }
        return null;
    }

    protected List o_() {
        return null;
    }

    private void a(ahb ahb2) {
        if (this.e == null) {
            this.e = (avj)ahb2.a(avj.class, this.a());
            if (this.e == null) {
                this.e = new avj(this.a());
                ahb2.a(this.a(), this.e);
            } else {
                dh dh2 = this.e.a();
                for (String string : dh2.c()) {
                    dh dh3;
                    dy dy2 = dh2.a(string);
                    if (dy2.a() != 10 || !(dh3 = (dh)dy2).c("ChunkX") || !dh3.c("ChunkZ")) continue;
                    int n2 = dh3.f("ChunkX");
                    int n3 = dh3.f("ChunkZ");
                    avm avm2 = avi.a(dh3, ahb2);
                    if (avm2 == null) continue;
                    this.d.put(agu.a(n2, n3), avm2);
                }
            }
        }
    }

    private void a(int n2, int n3, avm avm2) {
        this.e.a(avm2.a(n2, n3), n2, n3);
        this.e.c();
    }

    protected abstract boolean a(int var1, int var2);

    protected abstract avm b(int var1, int var2);
}

