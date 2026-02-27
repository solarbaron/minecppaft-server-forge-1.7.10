/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class any
extends alj {
    public static final String[] b = new String[]{"stone", "sand", "wood", "cobble", "brick", "smoothStoneBrick", "netherBrick", "quartz"};

    public any(boolean bl2) {
        super(bl2, awt.e);
        this.a(abt.b);
    }

    @Override
    public adb a(int n2, Random random, int n3) {
        return adb.a(ajn.U);
    }

    @Override
    protected add j(int n2) {
        return new add(adb.a(ajn.U), 2, n2 & 7);
    }

    @Override
    public String b(int n2) {
        if (n2 < 0 || n2 >= b.length) {
            n2 = 0;
        }
        return super.a() + "." + b[n2];
    }
}

