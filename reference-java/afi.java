/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.List;

public class afi
implements afg {
    private final add a;
    private final List b;

    public afi(add add2, List list) {
        this.a = add2;
        this.b = list;
    }

    @Override
    public add b() {
        return this.a;
    }

    @Override
    public boolean a(aae aae2, ahb ahb2) {
        ArrayList arrayList = new ArrayList(this.b);
        for (int i2 = 0; i2 < 3; ++i2) {
            for (int i3 = 0; i3 < 3; ++i3) {
                add add2 = aae2.b(i3, i2);
                if (add2 == null) continue;
                boolean bl2 = false;
                for (add add3 : arrayList) {
                    if (add2.b() != add3.b() || add3.k() != Short.MAX_VALUE && add2.k() != add3.k()) continue;
                    bl2 = true;
                    arrayList.remove(add3);
                    break;
                }
                if (bl2) continue;
                return false;
            }
        }
        return arrayList.isEmpty();
    }

    @Override
    public add a(aae aae2) {
        return this.a.m();
    }

    @Override
    public int a() {
        return this.b.size();
    }
}

