/*
 * Decompiled with CFR 0.152.
 */
import com.google.common.collect.Multimap;
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

public abstract class tl {
    protected final Map a = new HashMap();
    protected final Map b = new py();

    public ti a(th th2) {
        return (ti)this.a.get(th2);
    }

    public ti a(String string) {
        return (ti)this.b.get(string);
    }

    public abstract ti b(th var1);

    public Collection a() {
        return this.b.values();
    }

    public void a(tn tn2) {
    }

    public void a(Multimap multimap) {
        for (Map.Entry entry : multimap.entries()) {
            ti ti2 = this.a((String)entry.getKey());
            if (ti2 == null) continue;
            ti2.b((tj)entry.getValue());
        }
    }

    public void b(Multimap multimap) {
        for (Map.Entry entry : multimap.entries()) {
            ti ti2 = this.a((String)entry.getKey());
            if (ti2 == null) continue;
            ti2.b((tj)entry.getValue());
            ti2.a((tj)entry.getValue());
        }
    }
}

