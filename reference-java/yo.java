/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class yo
implements sy {
    public int a;

    public void a(Random random) {
        int n2 = random.nextInt(5);
        if (n2 <= 1) {
            this.a = rv.c.H;
        } else if (n2 <= 2) {
            this.a = rv.g.H;
        } else if (n2 <= 3) {
            this.a = rv.l.H;
        } else if (n2 <= 4) {
            this.a = rv.p.H;
        }
    }
}

