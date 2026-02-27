/*
 * Decompiled with CFR 0.152.
 */
import com.google.common.collect.Iterators;
import com.google.common.collect.Lists;
import java.util.Iterator;
import java.util.List;

public abstract class fe
implements fj {
    protected List a = Lists.newArrayList();
    private fn b;

    @Override
    public fj a(fj fj2) {
        fj2.b().a(this.b());
        this.a.add(fj2);
        return this;
    }

    @Override
    public List a() {
        return this.a;
    }

    @Override
    public fj a(String string) {
        return this.a(new fq(string));
    }

    @Override
    public fj a(fn fn2) {
        this.b = fn2;
        for (fj fj2 : this.a) {
            fj2.b().a(this.b());
        }
        return this;
    }

    @Override
    public fn b() {
        if (this.b == null) {
            this.b = new fn();
            for (fj fj2 : this.a) {
                fj2.b().a(this.b);
            }
        }
        return this.b;
    }

    public Iterator iterator() {
        return Iterators.concat(Iterators.forArray(this), fe.a(this.a));
    }

    @Override
    public final String c() {
        StringBuilder stringBuilder = new StringBuilder();
        for (fj fj2 : this) {
            stringBuilder.append(fj2.e());
        }
        return stringBuilder.toString();
    }

    public static Iterator a(Iterable iterable) {
        Iterator iterator = Iterators.concat(Iterators.transform(iterable.iterator(), new ff()));
        iterator = Iterators.transform(iterator, new fg());
        return iterator;
    }

    public boolean equals(Object object) {
        if (this == object) {
            return true;
        }
        if (object instanceof fe) {
            fe fe2 = (fe)object;
            return this.a.equals(fe2.a) && this.b().equals(fe2.b());
        }
        return false;
    }

    public int hashCode() {
        return 31 * this.b.hashCode() + this.a.hashCode();
    }

    public String toString() {
        return "BaseComponent{style=" + this.b + ", siblings=" + this.a + '}';
    }
}

