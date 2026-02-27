/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.List;

public class aps
extends aor {
    private aji a;
    private int i;
    private int j;
    private boolean k;
    private boolean l;
    private float m;
    private float n;
    private List o = new ArrayList();

    public aps() {
    }

    public aps(aji aji2, int n2, int n3, boolean bl2, boolean bl3) {
        this.a = aji2;
        this.i = n2;
        this.j = n3;
        this.k = bl2;
        this.l = bl3;
    }

    public aji a() {
        return this.a;
    }

    @Override
    public int p() {
        return this.i;
    }

    public boolean b() {
        return this.k;
    }

    public int c() {
        return this.j;
    }

    public float a(float f2) {
        if (f2 > 1.0f) {
            f2 = 1.0f;
        }
        return this.n + (this.m - this.n) * f2;
    }

    private void a(float f2, float f3) {
        List list;
        f2 = this.k ? 1.0f - f2 : (f2 -= 1.0f);
        azt azt2 = ajn.M.a(this.b, this.c, this.d, this.e, this.a, f2, this.j);
        if (azt2 != null && !(list = this.b.b(null, azt2)).isEmpty()) {
            this.o.addAll(list);
            for (sa sa2 : this.o) {
                sa2.d(f3 * (float)q.b[this.j], f3 * (float)q.c[this.j], f3 * (float)q.d[this.j]);
            }
            this.o.clear();
        }
    }

    public void f() {
        if (this.n < 1.0f && this.b != null) {
            this.m = 1.0f;
            this.n = 1.0f;
            this.b.p(this.c, this.d, this.e);
            this.s();
            if (this.b.a(this.c, this.d, this.e) == ajn.M) {
                this.b.d(this.c, this.d, this.e, this.a, this.i, 3);
                this.b.e(this.c, this.d, this.e, this.a);
            }
        }
    }

    @Override
    public void h() {
        this.n = this.m;
        if (this.n >= 1.0f) {
            this.a(1.0f, 0.25f);
            this.b.p(this.c, this.d, this.e);
            this.s();
            if (this.b.a(this.c, this.d, this.e) == ajn.M) {
                this.b.d(this.c, this.d, this.e, this.a, this.i, 3);
                this.b.e(this.c, this.d, this.e, this.a);
            }
            return;
        }
        this.m += 0.5f;
        if (this.m >= 1.0f) {
            this.m = 1.0f;
        }
        if (this.k) {
            this.a(this.m, this.m - this.n + 0.0625f);
        }
    }

    @Override
    public void a(dh dh2) {
        super.a(dh2);
        this.a = aji.e(dh2.f("blockId"));
        this.i = dh2.f("blockData");
        this.j = dh2.f("facing");
        this.n = this.m = dh2.h("progress");
        this.k = dh2.n("extending");
    }

    @Override
    public void b(dh dh2) {
        super.b(dh2);
        dh2.a("blockId", aji.b(this.a));
        dh2.a("blockData", this.i);
        dh2.a("facing", this.j);
        dh2.a("progress", this.n);
        dh2.a("extending", this.k);
    }
}

