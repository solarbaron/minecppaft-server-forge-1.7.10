/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;

public class bag
extends baf {
    public bag(String string) {
        super(string);
    }

    @Override
    public int a(List list) {
        float f2 = 0.0f;
        for (yz yz2 : list) {
            f2 += yz2.aS() + yz2.bs();
        }
        if (list.size() > 0) {
            f2 /= (float)list.size();
        }
        return qh.f(f2);
    }

    @Override
    public boolean b() {
        return true;
    }
}

