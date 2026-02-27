/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class auh
extends avm {
    public auh() {
    }

    public auh(ahb ahb2, Random random, int n2, int n3) {
        super(n2, n3);
        aui.b();
        auz auz2 = new auz(0, random, (n2 << 4) + 2, (n3 << 4) + 2);
        this.a.add(auz2);
        auz2.a(auz2, this.a, random);
        List list = auz2.c;
        while (!list.isEmpty()) {
            int n4 = random.nextInt(list.size());
            avk avk2 = (avk)list.remove(n4);
            avk2.a(auz2, this.a, random);
        }
        this.c();
        this.a(ahb2, random, 10);
    }
}

