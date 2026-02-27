/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public abstract class arn {
    private final boolean a;

    public arn() {
        this.a = false;
    }

    public arn(boolean bl2) {
        this.a = bl2;
    }

    public abstract boolean a(ahb var1, Random var2, int var3, int var4, int var5);

    public void a(double d2, double d3, double d4) {
    }

    protected void a(ahb ahb2, int n2, int n3, int n4, aji aji2) {
        this.a(ahb2, n2, n3, n4, aji2, 0);
    }

    protected void a(ahb ahb2, int n2, int n3, int n4, aji aji2, int n5) {
        if (this.a) {
            ahb2.d(n2, n3, n4, aji2, n5, 3);
        } else {
            ahb2.d(n2, n3, n4, aji2, n5, 2);
        }
    }
}

