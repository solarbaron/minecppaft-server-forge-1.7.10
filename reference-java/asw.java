/*
 * Decompiled with CFR 0.152.
 */
import java.util.Map;

public class asw
extends ave {
    private double e = 0.004;

    public asw() {
    }

    @Override
    public String a() {
        return "Mineshaft";
    }

    public asw(Map map) {
        for (Map.Entry entry : map.entrySet()) {
            if (!((String)entry.getKey()).equals("chance")) continue;
            this.e = qh.a((String)entry.getValue(), this.e);
        }
    }

    @Override
    protected boolean a(int n2, int n3) {
        return this.b.nextDouble() < this.e && this.b.nextInt(80) < Math.max(Math.abs(n2), Math.abs(n3));
    }

    @Override
    protected avm b(int n2, int n3) {
        return new atc(this.c, this.b, n2, n3);
    }
}

