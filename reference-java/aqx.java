/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class aqx {
    protected int a = 8;
    protected Random b = new Random();
    protected ahb c;

    public void a(apu apu2, ahb ahb2, int n2, int n3, aji[] ajiArray) {
        int n4 = this.a;
        this.c = ahb2;
        this.b.setSeed(ahb2.H());
        long l2 = this.b.nextLong();
        long l3 = this.b.nextLong();
        for (int i2 = n2 - n4; i2 <= n2 + n4; ++i2) {
            for (int i3 = n3 - n4; i3 <= n3 + n4; ++i3) {
                long l4 = (long)i2 * l2;
                long l5 = (long)i3 * l3;
                this.b.setSeed(l4 ^ l5 ^ ahb2.H());
                this.a(ahb2, i2, i3, n2, n3, ajiArray);
            }
        }
    }

    protected void a(ahb ahb2, int n2, int n3, int n4, int n5, aji[] ajiArray) {
    }
}

