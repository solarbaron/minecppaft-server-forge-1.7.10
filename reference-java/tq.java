/*
 * Decompiled with CFR 0.152.
 */
import com.google.common.collect.Sets;
import java.util.Collection;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

public class tq
extends tl {
    private final Set d = Sets.newHashSet();
    protected final Map c = new py();

    public tn c(th th2) {
        return (tn)super.a(th2);
    }

    public tn b(String string) {
        ti ti2 = super.a(string);
        if (ti2 == null) {
            ti2 = (ti)this.c.get(string);
        }
        return (tn)ti2;
    }

    @Override
    public ti b(th th2) {
        if (this.b.containsKey(th2.a())) {
            throw new IllegalArgumentException("Attribute is already registered!");
        }
        tn tn2 = new tn(this, th2);
        this.b.put(th2.a(), tn2);
        if (th2 instanceof to && ((to)th2).f() != null) {
            this.c.put(((to)th2).f(), tn2);
        }
        this.a.put(th2, tn2);
        return tn2;
    }

    @Override
    public void a(tn tn2) {
        if (tn2.a().c()) {
            this.d.add(tn2);
        }
    }

    public Set b() {
        return this.d;
    }

    public Collection c() {
        HashSet<ti> hashSet = Sets.newHashSet();
        for (ti ti2 : this.a()) {
            if (!ti2.a().c()) continue;
            hashSet.add(ti2);
        }
        return hashSet;
    }

    @Override
    public /* synthetic */ ti a(String string) {
        return this.b(string);
    }

    @Override
    public /* synthetic */ ti a(th th2) {
        return this.c(th2);
    }
}

