/*
 * Decompiled with CFR 0.152.
 */
import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.util.Random;

public class avn
extends ave {
    public static final List e = Arrays.asList(ahu.p, ahu.q, ahu.X);
    private int f;
    private int g = 32;
    private int h = 8;

    public avn() {
    }

    public avn(Map map) {
        this();
        for (Map.Entry entry : map.entrySet()) {
            if (((String)entry.getKey()).equals("size")) {
                this.f = qh.a((String)entry.getValue(), this.f, 0);
                continue;
            }
            if (!((String)entry.getKey()).equals("distance")) continue;
            this.g = qh.a((String)entry.getValue(), this.g, this.h + 1);
        }
    }

    @Override
    public String a() {
        return "Village";
    }

    @Override
    protected boolean a(int n2, int n3) {
        boolean bl2;
        int n4 = n2;
        int n5 = n3;
        if (n2 < 0) {
            n2 -= this.g - 1;
        }
        if (n3 < 0) {
            n3 -= this.g - 1;
        }
        int n6 = n2 / this.g;
        int n7 = n3 / this.g;
        Random random = this.c.A(n6, n7, 10387312);
        n6 *= this.g;
        n7 *= this.g;
        n2 = n4;
        n3 = n5;
        return n2 == (n6 += random.nextInt(this.g - this.h)) && n3 == (n7 += random.nextInt(this.g - this.h)) && (bl2 = this.c.v().a(n2 * 16 + 8, n3 * 16 + 8, 0, e));
    }

    @Override
    protected avm b(int n2, int n3) {
        return new avo(this.c, this.b, n2, n3, this.f);
    }
}

