/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.Random;

public class ate
extends avm {
    public ate() {
    }

    public ate(ahb ahb2, Random random, int n2, int n3) {
        super(n2, n3);
        atw atw2 = new atw(random, (n2 << 4) + 2, (n3 << 4) + 2);
        this.a.add(atw2);
        atw2.a(atw2, this.a, random);
        ArrayList arrayList = atw2.e;
        while (!arrayList.isEmpty()) {
            int n4 = random.nextInt(arrayList.size());
            avk avk2 = (avk)arrayList.remove(n4);
            avk2.a(atw2, this.a, random);
        }
        this.c();
        this.a(ahb2, random, 48, 70);
    }
}

