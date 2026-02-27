/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class aku
extends aji {
    public aku() {
        super(awt.e);
    }

    @Override
    public boolean c() {
        return false;
    }

    @Override
    public int b() {
        return 26;
    }

    @Override
    public void g() {
        this.a(0.0f, 0.0f, 0.0f, 1.0f, 0.8125f, 1.0f);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, azt azt2, List list, sa sa2) {
        this.a(0.0f, 0.0f, 0.0f, 1.0f, 0.8125f, 1.0f);
        super.a(ahb2, n2, n3, n4, azt2, list, sa2);
        int n5 = ahb2.e(n2, n3, n4);
        if (aku.b(n5)) {
            this.a(0.3125f, 0.8125f, 0.3125f, 0.6875f, 1.0f, 0.6875f);
            super.a(ahb2, n2, n3, n4, azt2, list, sa2);
        }
        this.g();
    }

    public static boolean b(int n2) {
        return (n2 & 4) != 0;
    }

    @Override
    public adb a(int n2, Random random, int n3) {
        return null;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, sv sv2, add add2) {
        int n5 = ((qh.c((double)(sv2.y * 4.0f / 360.0f) + 0.5) & 3) + 2) % 4;
        ahb2.a(n2, n3, n4, n5, 2);
    }

    @Override
    public boolean M() {
        return true;
    }

    @Override
    public int g(ahb ahb2, int n2, int n3, int n4, int n5) {
        int n6 = ahb2.e(n2, n3, n4);
        if (aku.b(n6)) {
            return 15;
        }
        return 0;
    }
}

