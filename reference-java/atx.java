/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.util.Random;

public class atx
extends ave {
    private static List e = Arrays.asList(ahu.q, ahu.F, ahu.J, ahu.K, ahu.u);
    private List f = new ArrayList();
    private int g = 32;
    private int h = 8;

    public atx() {
        this.f.add(new ahx(yp.class, 1, 1, 1));
    }

    public atx(Map map) {
        this();
        for (Map.Entry entry : map.entrySet()) {
            if (!((String)entry.getKey()).equals("distance")) continue;
            this.g = qh.a((String)entry.getValue(), this.g, this.h + 1);
        }
    }

    @Override
    public String a() {
        return "Temple";
    }

    @Override
    protected boolean a(int n2, int n3) {
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
        Random random = this.c.A(n6, n7, 14357617);
        n6 *= this.g;
        n7 *= this.g;
        n2 = n4;
        n3 = n5;
        if (n2 == (n6 += random.nextInt(this.g - this.h)) && n3 == (n7 += random.nextInt(this.g - this.h))) {
            ahu ahu2 = this.c.v().a(n2 * 16 + 8, n3 * 16 + 8);
            for (ahu ahu3 : e) {
                if (ahu2 != ahu3) continue;
                return true;
            }
        }
        return false;
    }

    @Override
    protected avm b(int n2, int n3) {
        return new aty(this.c, this.b, n2, n3);
    }

    public boolean a(int n2, int n3, int n4) {
        avm avm2 = this.c(n2, n3, n4);
        if (avm2 == null || !(avm2 instanceof aty) || avm2.a.isEmpty()) {
            return false;
        }
        avk avk2 = (avk)avm2.a.getFirst();
        return avk2 instanceof auf;
    }

    public List b() {
        return this.f;
    }
}

