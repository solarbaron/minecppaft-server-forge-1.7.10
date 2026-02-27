/*
 * Decompiled with CFR 0.152.
 */
import java.util.Collections;
import java.util.List;

public class vo
extends vu {
    private final Class a;
    private final int b;
    private final vq e;
    private final sj f;
    private sv g;

    public vo(td td2, Class clazz, int n2, boolean bl2) {
        this(td2, clazz, n2, bl2, false);
    }

    public vo(td td2, Class clazz, int n2, boolean bl2, boolean bl3) {
        this(td2, clazz, n2, bl2, bl3, null);
    }

    public vo(td td2, Class clazz, int n2, boolean bl2, boolean bl3, sj sj2) {
        super(td2, bl2, bl3);
        this.a = clazz;
        this.b = n2;
        this.e = new vq(td2);
        this.a(1);
        this.f = new vp(this, sj2);
    }

    @Override
    public boolean a() {
        if (this.b > 0 && this.c.aI().nextInt(this.b) != 0) {
            return false;
        }
        double d2 = this.f();
        List list = this.c.o.a(this.a, this.c.C.b(d2, 4.0, d2), this.f);
        Collections.sort(list, this.e);
        if (list.isEmpty()) {
            return false;
        }
        this.g = (sv)list.get(0);
        return true;
    }

    @Override
    public void c() {
        this.c.d(this.g);
        super.c();
    }
}

