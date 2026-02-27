/*
 * Decompiled with CFR 0.152.
 */
import com.google.common.collect.Maps;
import java.util.Map;

public class gd
extends ft {
    private Map a;

    public gd() {
    }

    public gd(Map map) {
        this.a = map;
    }

    public void a(fv fv2) {
        fv2.a(this);
    }

    @Override
    public void a(et et2) {
        int n2 = et2.a();
        this.a = Maps.newHashMap();
        for (int i2 = 0; i2 < n2; ++i2) {
            ph ph2 = pp.a(et2.c(Short.MAX_VALUE));
            int n3 = et2.a();
            if (ph2 == null) continue;
            this.a.put(ph2, n3);
        }
    }

    @Override
    public void b(et et2) {
        et2.b(this.a.size());
        for (Map.Entry entry : this.a.entrySet()) {
            et2.a(((ph)entry.getKey()).e);
            et2.b((Integer)entry.getValue());
        }
    }

    @Override
    public String b() {
        return String.format("count=%d", this.a.size());
    }
}

