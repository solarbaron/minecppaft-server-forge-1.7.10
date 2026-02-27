/*
 * Decompiled with CFR 0.152.
 */
import java.text.DecimalFormat;
import java.text.NumberFormat;
import java.util.Locale;

public class ph {
    public final String e;
    private final fj a;
    public boolean f;
    private final pm b;
    private final bah c;
    private Class d;
    private static NumberFormat k = NumberFormat.getIntegerInstance(Locale.US);
    public static pm g = new pi();
    private static DecimalFormat l = new DecimalFormat("########0.00");
    public static pm h = new pj();
    public static pm i = new pk();
    public static pm j = new pl();

    public ph(String string, fj fj2, pm pm2) {
        this.e = string;
        this.a = fj2;
        this.b = pm2;
        this.c = new bai(this);
        bah.a.put(this.c.a(), this.c);
    }

    public ph(String string, fj fj2) {
        this(string, fj2, g);
    }

    public ph i() {
        this.f = true;
        return this;
    }

    public ph h() {
        if (pp.a.containsKey(this.e)) {
            throw new RuntimeException("Duplicate stat id: \"" + ((ph)pp.a.get((Object)this.e)).a + "\" and \"" + this.a + "\" at id " + this.e);
        }
        pp.b.add(this);
        pp.a.put(this.e, this);
        return this;
    }

    public boolean d() {
        return false;
    }

    public fj e() {
        fj fj2 = this.a.f();
        fj2.b().a(a.h);
        fj2.b().a(new fl(fm.b, new fq(this.e)));
        return fj2;
    }

    public fj j() {
        fj fj2 = this.e();
        fj fj3 = new fq("[").a(fj2).a("]");
        fj3.a(fj2.b());
        return fj3;
    }

    public boolean equals(Object object) {
        if (this == object) {
            return true;
        }
        if (object == null || this.getClass() != object.getClass()) {
            return false;
        }
        ph ph2 = (ph)object;
        return this.e.equals(ph2.e);
    }

    public int hashCode() {
        return this.e.hashCode();
    }

    public String toString() {
        return "Stat{id=" + this.e + ", nameId=" + this.a + ", awardLocallyOnly=" + this.f + ", formatter=" + this.b + ", objectiveCriteria=" + this.c + '}';
    }

    public bah k() {
        return this.c;
    }

    public Class l() {
        return this.d;
    }

    public ph b(Class clazz) {
        this.d = clazz;
        return this;
    }
}

