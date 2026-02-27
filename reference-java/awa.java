/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.List;
import java.util.Random;

public class awa
extends awf {
    public aib a;
    public boolean b;
    public int c;
    public avu d;
    public List e;
    public List i = new ArrayList();
    public List j = new ArrayList();

    public awa() {
    }

    public awa(aib aib2, int n2, Random random, int n3, int n4, List list, int n5) {
        super(null, 0, random, n3, n4);
        this.a = aib2;
        this.e = list;
        this.c = n5;
        ahu ahu2 = aib2.a(n3, n4);
        this.b = ahu2 == ahu.q || ahu2 == ahu.F;
    }

    public aib e() {
        return this.a;
    }
}

