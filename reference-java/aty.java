/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class aty
extends avm {
    public aty() {
    }

    public aty(ahb ahb2, Random random, int n2, int n3) {
        super(n2, n3);
        ahu ahu2 = ahb2.a(n2 * 16 + 8, n3 * 16 + 8);
        if (ahu2 == ahu.J || ahu2 == ahu.K) {
            auc auc2 = new auc(random, n2 * 16, n3 * 16);
            this.a.add(auc2);
        } else if (ahu2 == ahu.u) {
            auf auf2 = new auf(random, n2 * 16, n3 * 16);
            this.a.add(auf2);
        } else {
            aub aub2 = new aub(random, n2 * 16, n3 * 16);
            this.a.add(aub2);
        }
        this.c();
    }
}

