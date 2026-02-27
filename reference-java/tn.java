/*
 * Decompiled with CFR 0.152.
 */
import com.google.common.collect.Maps;
import java.util.Collection;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.UUID;

public class tn
implements ti {
    private final tl a;
    private final th b;
    private final Map c = Maps.newHashMap();
    private final Map d = Maps.newHashMap();
    private final Map e = Maps.newHashMap();
    private double f;
    private boolean g = true;
    private double h;

    public tn(tl tl2, th th2) {
        this.a = tl2;
        this.b = th2;
        this.f = th2.b();
        for (int i2 = 0; i2 < 3; ++i2) {
            this.c.put(i2, new HashSet());
        }
    }

    @Override
    public th a() {
        return this.b;
    }

    @Override
    public double b() {
        return this.f;
    }

    @Override
    public void a(double d2) {
        if (d2 == this.b()) {
            return;
        }
        this.f = d2;
        this.f();
    }

    public Collection a(int n2) {
        return (Collection)this.c.get(n2);
    }

    @Override
    public Collection c() {
        HashSet hashSet = new HashSet();
        for (int i2 = 0; i2 < 3; ++i2) {
            hashSet.addAll(this.a(i2));
        }
        return hashSet;
    }

    @Override
    public tj a(UUID uUID) {
        return (tj)this.e.get(uUID);
    }

    @Override
    public void a(tj tj2) {
        if (this.a(tj2.a()) != null) {
            throw new IllegalArgumentException("Modifier is already applied on this attribute!");
        }
        HashSet<tj> hashSet = (HashSet<tj>)this.d.get(tj2.b());
        if (hashSet == null) {
            hashSet = new HashSet<tj>();
            this.d.put(tj2.b(), hashSet);
        }
        ((Set)this.c.get(tj2.c())).add(tj2);
        hashSet.add(tj2);
        this.e.put(tj2.a(), tj2);
        this.f();
    }

    private void f() {
        this.g = true;
        this.a.a(this);
    }

    @Override
    public void b(tj tj2) {
        for (int i2 = 0; i2 < 3; ++i2) {
            Set set = (Set)this.c.get(i2);
            set.remove(tj2);
        }
        Set set = (Set)this.d.get(tj2.b());
        if (set != null) {
            set.remove(tj2);
            if (set.isEmpty()) {
                this.d.remove(tj2.b());
            }
        }
        this.e.remove(tj2.a());
        this.f();
    }

    @Override
    public double e() {
        if (this.g) {
            this.h = this.g();
            this.g = false;
        }
        return this.h;
    }

    private double g() {
        double d2 = this.b();
        for (tj tj2 : this.a(0)) {
            d2 += tj2.d();
        }
        double d3 = d2;
        for (tj tj3 : this.a(1)) {
            d3 += d2 * tj3.d();
        }
        for (tj tj3 : this.a(2)) {
            d3 *= 1.0 + tj3.d();
        }
        return this.b.a(d3);
    }
}

