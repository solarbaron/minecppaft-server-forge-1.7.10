/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.List;

class mr {
    private final List b = new ArrayList();
    private final agu c;
    private short[] d = new short[64];
    private int e;
    private int f;
    private long g;
    final /* synthetic */ mq a;

    public mr(mq mq2, int n2, int n3) {
        this.a = mq2;
        this.c = new agu(n2, n3);
        mq2.a().b.c(n2, n3);
    }

    public void a(mw mw2) {
        if (this.b.contains(mw2)) {
            mq.c().debug("Failed to add player. {} already is in chunk {}, {}", mw2, this.c.a, this.c.b);
            return;
        }
        if (this.b.isEmpty()) {
            this.g = mq.a(this.a).I();
        }
        this.b.add(mw2);
        mw2.f.add(this.c);
    }

    public void b(mw mw2) {
        if (!this.b.contains(mw2)) {
            return;
        }
        apx apx2 = mq.a(this.a).e(this.c.a, this.c.b);
        if (apx2.k()) {
            mw2.a.a(new gx(apx2, true, 0));
        }
        this.b.remove(mw2);
        mw2.f.remove(this.c);
        if (this.b.isEmpty()) {
            long l2 = (long)this.c.a + Integer.MAX_VALUE | (long)this.c.b + Integer.MAX_VALUE << 32;
            this.a(apx2);
            mq.b(this.a).d(l2);
            mq.c(this.a).remove(this);
            if (this.e > 0) {
                mq.d(this.a).remove(this);
            }
            this.a.a().b.b(this.c.a, this.c.b);
        }
    }

    public void a() {
        this.a(mq.a(this.a).e(this.c.a, this.c.b));
    }

    private void a(apx apx2) {
        apx2.s += mq.a(this.a).I() - this.g;
        this.g = mq.a(this.a).I();
    }

    public void a(int n2, int n3, int n4) {
        if (this.e == 0) {
            mq.d(this.a).add(this);
        }
        this.f |= 1 << (n3 >> 4);
        if (this.e < 64) {
            short s2 = (short)(n2 << 12 | n4 << 8 | n3);
            for (int i2 = 0; i2 < this.e; ++i2) {
                if (this.d[i2] != s2) continue;
                return;
            }
            this.d[this.e++] = s2;
        }
    }

    public void a(ft ft2) {
        for (int i2 = 0; i2 < this.b.size(); ++i2) {
            mw mw2 = (mw)this.b.get(i2);
            if (mw2.f.contains(this.c)) continue;
            mw2.a.a(ft2);
        }
    }

    public void b() {
        if (this.e == 0) {
            return;
        }
        if (this.e == 1) {
            int n2 = this.c.a * 16 + (this.d[0] >> 12 & 0xF);
            int n3 = this.d[0] & 0xFF;
            int n4 = this.c.b * 16 + (this.d[0] >> 8 & 0xF);
            this.a(new gh(n2, n3, n4, mq.a(this.a)));
            if (mq.a(this.a).a(n2, n3, n4).u()) {
                this.a(mq.a(this.a).o(n2, n3, n4));
            }
        } else if (this.e == 64) {
            int n5 = this.c.a * 16;
            int n6 = this.c.b * 16;
            this.a(new gx(mq.a(this.a).e(this.c.a, this.c.b), false, this.f));
            for (int i2 = 0; i2 < 16; ++i2) {
                if ((this.f & 1 << i2) == 0) continue;
                int n7 = i2 << 4;
                List list = mq.a(this.a).a(n5, n7, n6, n5 + 16, n7 + 16, n6 + 16);
                for (int i3 = 0; i3 < list.size(); ++i3) {
                    this.a((aor)list.get(i3));
                }
            }
        } else {
            this.a(new gk(this.e, this.d, mq.a(this.a).e(this.c.a, this.c.b)));
            for (int i4 = 0; i4 < this.e; ++i4) {
                int n8 = this.c.a * 16 + (this.d[i4] >> 12 & 0xF);
                int n9 = this.d[i4] & 0xFF;
                int n10 = this.c.b * 16 + (this.d[i4] >> 8 & 0xF);
                if (!mq.a(this.a).a(n8, n9, n10).u()) continue;
                this.a(mq.a(this.a).o(n8, n9, n10));
            }
        }
        this.e = 0;
        this.f = 0;
    }

    private void a(aor aor2) {
        ft ft2;
        if (aor2 != null && (ft2 = aor2.m()) != null) {
            this.a(ft2);
        }
    }

    static /* synthetic */ agu a(mr mr2) {
        return mr2.c;
    }

    static /* synthetic */ List b(mr mr2) {
        return mr2.b;
    }
}

