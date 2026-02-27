/*
 * Decompiled with CFR 0.152.
 */
import com.google.common.collect.Iterators;
import com.google.common.collect.Lists;
import java.util.Arrays;
import java.util.IllegalFormatException;
import java.util.Iterator;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class fr
extends fe {
    private final String d;
    private final Object[] e;
    private final Object f = new Object();
    private long g = -1L;
    List b = Lists.newArrayList();
    public static final Pattern c = Pattern.compile("%(?:(\\d+)\\$)?([A-Za-z%]|$)");

    public fr(String string, Object ... objectArray) {
        this.d = string;
        this.e = objectArray;
        for (Object object : objectArray) {
            if (!(object instanceof fj)) continue;
            ((fj)object).b().a(this.b());
        }
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    synchronized void g() {
        Object object = this.f;
        synchronized (object) {
            long l2 = dd.a();
            if (l2 == this.g) {
                return;
            }
            this.g = l2;
            this.b.clear();
        }
        try {
            this.b(dd.a(this.d));
        }
        catch (fs fs2) {
            this.b.clear();
            try {
                this.b(dd.b(this.d));
            }
            catch (fs fs3) {
                throw fs2;
            }
        }
    }

    protected void b(String string) {
        boolean bl2 = false;
        Matcher matcher = c.matcher(string);
        int n2 = 0;
        int n3 = 0;
        try {
            while (matcher.find(n3)) {
                Object object;
                Object object2;
                int n4 = matcher.start();
                int n5 = matcher.end();
                if (n4 > n3) {
                    object2 = new fq(String.format(string.substring(n3, n4), new Object[0]));
                    ((fe)object2).b().a(this.b());
                    this.b.add(object2);
                }
                object2 = matcher.group(2);
                String string2 = string.substring(n4, n5);
                if ("%".equals(object2) && "%%".equals(string2)) {
                    object = new fq("%");
                    ((fe)object).b().a(this.b());
                    this.b.add(object);
                } else if ("s".equals(object2)) {
                    object = matcher.group(1);
                    int n6 = object != null ? Integer.parseInt((String)object) - 1 : n2++;
                    this.b.add(this.a(n6));
                } else {
                    throw new fs(this, "Unsupported format: '" + string2 + "'");
                }
                n3 = n5;
            }
            if (n3 < string.length()) {
                fq fq2 = new fq(String.format(string.substring(n3), new Object[0]));
                fq2.b().a(this.b());
                this.b.add(fq2);
            }
        }
        catch (IllegalFormatException illegalFormatException) {
            throw new fs(this, (Throwable)illegalFormatException);
        }
    }

    private fj a(int n2) {
        fj fj2;
        if (n2 >= this.e.length) {
            throw new fs(this, n2);
        }
        Object object = this.e[n2];
        if (object instanceof fj) {
            fj2 = (fj)object;
        } else {
            fj2 = new fq(object == null ? "null" : object.toString());
            fj2.b().a(this.b());
        }
        return fj2;
    }

    @Override
    public fj a(fn fn2) {
        super.a(fn2);
        for (Object object : this.e) {
            if (!(object instanceof fj)) continue;
            ((fj)object).b().a(this.b());
        }
        if (this.g > -1L) {
            for (fj fj2 : this.b) {
                fj2.b().a(fn2);
            }
        }
        return this;
    }

    @Override
    public Iterator iterator() {
        this.g();
        return Iterators.concat(fr.a(this.b), fr.a(this.a));
    }

    @Override
    public String e() {
        this.g();
        StringBuilder stringBuilder = new StringBuilder();
        for (fj fj2 : this.b) {
            stringBuilder.append(fj2.e());
        }
        return stringBuilder.toString();
    }

    public fr h() {
        Object[] objectArray = new Object[this.e.length];
        for (int i2 = 0; i2 < this.e.length; ++i2) {
            objectArray[i2] = this.e[i2] instanceof fj ? ((fj)this.e[i2]).f() : this.e[i2];
        }
        fr fr2 = new fr(this.d, objectArray);
        fr2.a(this.b().l());
        for (fj fj2 : this.a()) {
            fr2.a(fj2.f());
        }
        return fr2;
    }

    @Override
    public boolean equals(Object object) {
        if (this == object) {
            return true;
        }
        if (object instanceof fr) {
            fr fr2 = (fr)object;
            return Arrays.equals(this.e, fr2.e) && this.d.equals(fr2.d) && super.equals(object);
        }
        return false;
    }

    @Override
    public int hashCode() {
        int n2 = super.hashCode();
        n2 = 31 * n2 + this.d.hashCode();
        n2 = 31 * n2 + Arrays.hashCode(this.e);
        return n2;
    }

    @Override
    public String toString() {
        return "TranslatableComponent{key='" + this.d + '\'' + ", args=" + Arrays.toString(this.e) + ", siblings=" + this.a + ", style=" + this.b() + '}';
    }

    public String i() {
        return this.d;
    }

    public Object[] j() {
        return this.e;
    }

    @Override
    public /* synthetic */ fj f() {
        return this.h();
    }
}

