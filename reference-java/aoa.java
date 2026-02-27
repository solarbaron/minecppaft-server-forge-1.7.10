/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class aoa
extends aji {
    private final String a;
    private final boolean b;
    private final String M;

    protected aoa(String string, String string2, awt awt2, boolean bl2) {
        super(awt2);
        this.a = string2;
        this.b = bl2;
        this.M = string;
        this.a(abt.c);
    }

    @Override
    public adb a(int n2, Random random, int n3) {
        if (!this.b) {
            return null;
        }
        return super.a(n2, random, n3);
    }

    @Override
    public boolean c() {
        return false;
    }

    @Override
    public boolean d() {
        return false;
    }

    @Override
    public int b() {
        return this.J == awt.s ? 41 : 18;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, azt azt2, List list, sa sa2) {
        boolean bl2 = this.a(ahb2.a(n2, n3, n4 - 1));
        boolean bl3 = this.a(ahb2.a(n2, n3, n4 + 1));
        boolean bl4 = this.a(ahb2.a(n2 - 1, n3, n4));
        boolean bl5 = this.a(ahb2.a(n2 + 1, n3, n4));
        if (bl4 && bl5 || !bl4 && !bl5 && !bl2 && !bl3) {
            this.a(0.0f, 0.0f, 0.4375f, 1.0f, 1.0f, 0.5625f);
            super.a(ahb2, n2, n3, n4, azt2, list, sa2);
        } else if (bl4 && !bl5) {
            this.a(0.0f, 0.0f, 0.4375f, 0.5f, 1.0f, 0.5625f);
            super.a(ahb2, n2, n3, n4, azt2, list, sa2);
        } else if (!bl4 && bl5) {
            this.a(0.5f, 0.0f, 0.4375f, 1.0f, 1.0f, 0.5625f);
            super.a(ahb2, n2, n3, n4, azt2, list, sa2);
        }
        if (bl2 && bl3 || !bl4 && !bl5 && !bl2 && !bl3) {
            this.a(0.4375f, 0.0f, 0.0f, 0.5625f, 1.0f, 1.0f);
            super.a(ahb2, n2, n3, n4, azt2, list, sa2);
        } else if (bl2 && !bl3) {
            this.a(0.4375f, 0.0f, 0.0f, 0.5625f, 1.0f, 0.5f);
            super.a(ahb2, n2, n3, n4, azt2, list, sa2);
        } else if (!bl2 && bl3) {
            this.a(0.4375f, 0.0f, 0.5f, 0.5625f, 1.0f, 1.0f);
            super.a(ahb2, n2, n3, n4, azt2, list, sa2);
        }
    }

    @Override
    public void g() {
        this.a(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    }

    @Override
    public void a(ahl ahl2, int n2, int n3, int n4) {
        float f2 = 0.4375f;
        float f3 = 0.5625f;
        float f4 = 0.4375f;
        float f5 = 0.5625f;
        boolean bl2 = this.a(ahl2.a(n2, n3, n4 - 1));
        boolean bl3 = this.a(ahl2.a(n2, n3, n4 + 1));
        boolean bl4 = this.a(ahl2.a(n2 - 1, n3, n4));
        boolean bl5 = this.a(ahl2.a(n2 + 1, n3, n4));
        if (bl4 && bl5 || !bl4 && !bl5 && !bl2 && !bl3) {
            f2 = 0.0f;
            f3 = 1.0f;
        } else if (bl4 && !bl5) {
            f2 = 0.0f;
        } else if (!bl4 && bl5) {
            f3 = 1.0f;
        }
        if (bl2 && bl3 || !bl4 && !bl5 && !bl2 && !bl3) {
            f4 = 0.0f;
            f5 = 1.0f;
        } else if (bl2 && !bl3) {
            f4 = 0.0f;
        } else if (!bl2 && bl3) {
            f5 = 1.0f;
        }
        this.a(f2, 0.0f, f4, f3, 1.0f, f5);
    }

    public final boolean a(aji aji2) {
        return aji2.j() || aji2 == this || aji2 == ajn.w || aji2 == ajn.cn || aji2 == ajn.co || aji2 instanceof aoa;
    }

    @Override
    protected boolean E() {
        return true;
    }

    @Override
    protected add j(int n2) {
        return new add(adb.a(this), 1, n2);
    }
}

