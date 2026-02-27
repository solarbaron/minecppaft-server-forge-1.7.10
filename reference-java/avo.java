/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class avo
extends avm {
    private boolean c;

    public avo() {
    }

    public avo(ahb ahb2, Random random, int n2, int n3, int n4) {
        super(n2, n3);
        int n5;
        List list = avp.a(random, n4);
        awa awa2 = new awa(ahb2.v(), 0, random, (n2 << 4) + 2, (n3 << 4) + 2, list, n4);
        this.a.add(awa2);
        awa2.a(awa2, this.a, random);
        List list2 = awa2.j;
        List list3 = awa2.i;
        while (!list2.isEmpty() || !list3.isEmpty()) {
            Object object;
            if (list2.isEmpty()) {
                n5 = random.nextInt(list3.size());
                object = (avk)list3.remove(n5);
                ((avk)object).a(awa2, this.a, random);
                continue;
            }
            n5 = random.nextInt(list2.size());
            object = (avk)list2.remove(n5);
            ((avk)object).a(awa2, this.a, random);
        }
        this.c();
        n5 = 0;
        for (avk avk2 : this.a) {
            if (avk2 instanceof awe) continue;
            ++n5;
        }
        this.c = n5 > 2;
    }

    @Override
    public boolean d() {
        return this.c;
    }

    @Override
    public void a(dh dh2) {
        super.a(dh2);
        dh2.a("Valid", this.c);
    }

    @Override
    public void b(dh dh2) {
        super.b(dh2);
        this.c = dh2.n("Valid");
    }
}

