/*
 * Decompiled with CFR 0.152.
 */
public class agg
extends aft {
    protected agg(int n2, int n3, afu afu2) {
        super(n2, n3, afu2);
        if (afu2 == afu.h) {
            this.b("lootBonusDigger");
        } else if (afu2 == afu.i) {
            this.b("lootBonusFishing");
        } else {
            this.b("lootBonus");
        }
    }

    @Override
    public int a(int n2) {
        return 15 + (n2 - 1) * 9;
    }

    @Override
    public int b(int n2) {
        return super.a(n2) + 50;
    }

    @Override
    public int b() {
        return 3;
    }

    @Override
    public boolean a(aft aft2) {
        return super.a(aft2) && aft2.B != agg.s.B;
    }
}

