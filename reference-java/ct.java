/*
 * Decompiled with CFR 0.152.
 */
import com.google.common.base.Predicates;
import com.google.common.collect.Iterators;
import com.google.common.collect.Lists;
import java.util.IdentityHashMap;
import java.util.Iterator;
import java.util.List;

public class ct
implements cs {
    private IdentityHashMap a = new IdentityHashMap(512);
    private List b = Lists.newArrayList();

    public void a(Object object, int n2) {
        this.a.put(object, n2);
        while (this.b.size() <= n2) {
            this.b.add(null);
        }
        this.b.set(n2, object);
    }

    public int b(Object object) {
        Integer n2 = (Integer)this.a.get(object);
        return n2 == null ? -1 : n2;
    }

    public Object a(int n2) {
        if (n2 >= 0 && n2 < this.b.size()) {
            return this.b.get(n2);
        }
        return null;
    }

    public Iterator iterator() {
        return Iterators.filter(this.b.iterator(), Predicates.notNull());
    }

    public boolean b(int n2) {
        return this.a(n2) != null;
    }
}

