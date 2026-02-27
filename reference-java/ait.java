/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class ait
extends ain {
    public ait(int n2, ahu ahu2) {
        super(n2, ahu2);
        this.ar.x = 2;
        this.ar.y = 2;
        this.ar.z = 5;
    }

    @Override
    public void a(ahb ahb2, Random random, aji[] ajiArray, byte[] byArray, int n2, int n3, double d2) {
        this.ai = ajn.c;
        this.aj = 0;
        this.ak = ajn.d;
        if (d2 > 1.75) {
            this.ai = ajn.b;
            this.ak = ajn.b;
        } else if (d2 > -0.5) {
            this.ai = ajn.d;
            this.aj = 1;
        }
        this.b(ahb2, random, ajiArray, byArray, n2, n3, d2);
    }

    @Override
    public void a(ahb ahb2, Random random, int n2, int n3) {
        this.ar.a(ahb2, random, this, n2, n3);
    }
}

