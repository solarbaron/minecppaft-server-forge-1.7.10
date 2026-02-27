/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.List;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class mq {
    private static final Logger a = LogManager.getLogger();
    private final mt b;
    private final List c = new ArrayList();
    private final qd d = new qd();
    private final List e = new ArrayList();
    private final List f = new ArrayList();
    private int g;
    private long h;
    private final int[][] i = new int[][]{{1, 0}, {0, 1}, {-1, 0}, {0, -1}};

    public mq(mt mt2) {
        this.b = mt2;
        this.a(mt2.q().ah().s());
    }

    public mt a() {
        return this.b;
    }

    public void b() {
        aqo aqo2;
        int n2;
        long l2 = this.b.I();
        if (l2 - this.h > 8000L) {
            this.h = l2;
            for (n2 = 0; n2 < this.f.size(); ++n2) {
                mr mr2 = (mr)this.f.get(n2);
                mr2.b();
                mr2.a();
            }
        } else {
            for (n2 = 0; n2 < this.e.size(); ++n2) {
                mr mr3 = (mr)this.e.get(n2);
                mr3.b();
            }
        }
        this.e.clear();
        if (this.c.isEmpty() && !(aqo2 = this.b.t).e()) {
            this.b.b.b();
        }
    }

    public boolean a(int n2, int n3) {
        long l2 = (long)n2 + Integer.MAX_VALUE | (long)n3 + Integer.MAX_VALUE << 32;
        return this.d.a(l2) != null;
    }

    private mr a(int n2, int n3, boolean bl2) {
        long l2 = (long)n2 + Integer.MAX_VALUE | (long)n3 + Integer.MAX_VALUE << 32;
        mr mr2 = (mr)this.d.a(l2);
        if (mr2 == null && bl2) {
            mr2 = new mr(this, n2, n3);
            this.d.a(l2, mr2);
            this.f.add(mr2);
        }
        return mr2;
    }

    public void a(int n2, int n3, int n4) {
        int n5 = n2 >> 4;
        int n6 = n4 >> 4;
        mr mr2 = this.a(n5, n6, false);
        if (mr2 != null) {
            mr2.a(n2 & 0xF, n3, n4 & 0xF);
        }
    }

    public void a(mw mw2) {
        int n2 = (int)mw2.s >> 4;
        int n3 = (int)mw2.u >> 4;
        mw2.d = mw2.s;
        mw2.e = mw2.u;
        for (int i2 = n2 - this.g; i2 <= n2 + this.g; ++i2) {
            for (int i3 = n3 - this.g; i3 <= n3 + this.g; ++i3) {
                this.a(i2, i3, true).a(mw2);
            }
        }
        this.c.add(mw2);
        this.b(mw2);
    }

    public void b(mw mw2) {
        int n2;
        ArrayList arrayList = new ArrayList(mw2.f);
        int n3 = 0;
        int n4 = this.g;
        int n5 = (int)mw2.s >> 4;
        int n6 = (int)mw2.u >> 4;
        int n7 = 0;
        int n8 = 0;
        agu agu2 = mr.a(this.a(n5, n6, true));
        mw2.f.clear();
        if (arrayList.contains(agu2)) {
            mw2.f.add(agu2);
        }
        for (n2 = 1; n2 <= n4 * 2; ++n2) {
            for (int i2 = 0; i2 < 2; ++i2) {
                int[] nArray = this.i[n3++ % 4];
                for (int i3 = 0; i3 < n2; ++i3) {
                    agu2 = mr.a(this.a(n5 + (n7 += nArray[0]), n6 + (n8 += nArray[1]), true));
                    if (!arrayList.contains(agu2)) continue;
                    mw2.f.add(agu2);
                }
            }
        }
        n3 %= 4;
        for (n2 = 0; n2 < n4 * 2; ++n2) {
            agu2 = mr.a(this.a(n5 + (n7 += this.i[n3][0]), n6 + (n8 += this.i[n3][1]), true));
            if (!arrayList.contains(agu2)) continue;
            mw2.f.add(agu2);
        }
    }

    public void c(mw mw2) {
        int n2 = (int)mw2.d >> 4;
        int n3 = (int)mw2.e >> 4;
        for (int i2 = n2 - this.g; i2 <= n2 + this.g; ++i2) {
            for (int i3 = n3 - this.g; i3 <= n3 + this.g; ++i3) {
                mr mr2 = this.a(i2, i3, false);
                if (mr2 == null) continue;
                mr2.b(mw2);
            }
        }
        this.c.remove(mw2);
    }

    private boolean a(int n2, int n3, int n4, int n5, int n6) {
        int n7 = n2 - n4;
        int n8 = n3 - n5;
        if (n7 < -n6 || n7 > n6) {
            return false;
        }
        return n8 >= -n6 && n8 <= n6;
    }

    public void d(mw mw2) {
        int n2 = (int)mw2.s >> 4;
        int n3 = (int)mw2.u >> 4;
        double d2 = mw2.d - mw2.s;
        double d3 = mw2.e - mw2.u;
        double d4 = d2 * d2 + d3 * d3;
        if (d4 < 64.0) {
            return;
        }
        int n4 = (int)mw2.d >> 4;
        int n5 = (int)mw2.e >> 4;
        int n6 = this.g;
        int n7 = n2 - n4;
        int n8 = n3 - n5;
        if (n7 == 0 && n8 == 0) {
            return;
        }
        for (int i2 = n2 - n6; i2 <= n2 + n6; ++i2) {
            for (int i3 = n3 - n6; i3 <= n3 + n6; ++i3) {
                mr mr2;
                if (!this.a(i2, i3, n4, n5, n6)) {
                    this.a(i2, i3, true).a(mw2);
                }
                if (this.a(i2 - n7, i3 - n8, n2, n3, n6) || (mr2 = this.a(i2 - n7, i3 - n8, false)) == null) continue;
                mr2.b(mw2);
            }
        }
        this.b(mw2);
        mw2.d = mw2.s;
        mw2.e = mw2.u;
    }

    public boolean a(mw mw2, int n2, int n3) {
        mr mr2 = this.a(n2, n3, false);
        return mr2 != null && mr.b(mr2).contains(mw2) && !mw2.f.contains(mr.a(mr2));
    }

    public void a(int n2) {
        if ((n2 = qh.a(n2, 3, 20)) == this.g) {
            return;
        }
        int n3 = n2 - this.g;
        for (mw mw2 : this.c) {
            int n4;
            int n5;
            int n6 = (int)mw2.s >> 4;
            int n7 = (int)mw2.u >> 4;
            if (n3 > 0) {
                for (n5 = n6 - n2; n5 <= n6 + n2; ++n5) {
                    for (n4 = n7 - n2; n4 <= n7 + n2; ++n4) {
                        mr mr2 = this.a(n5, n4, true);
                        if (mr.b(mr2).contains(mw2)) continue;
                        mr2.a(mw2);
                    }
                }
                continue;
            }
            for (n5 = n6 - this.g; n5 <= n6 + this.g; ++n5) {
                for (n4 = n7 - this.g; n4 <= n7 + this.g; ++n4) {
                    if (this.a(n5, n4, n6, n7, n2)) continue;
                    this.a(n5, n4, true).b(mw2);
                }
            }
        }
        this.g = n2;
    }

    public static int b(int n2) {
        return n2 * 16 - 16;
    }

    static /* synthetic */ Logger c() {
        return a;
    }

    static /* synthetic */ mt a(mq mq2) {
        return mq2.b;
    }

    static /* synthetic */ qd b(mq mq2) {
        return mq2.d;
    }

    static /* synthetic */ List c(mq mq2) {
        return mq2.f;
    }

    static /* synthetic */ List d(mq mq2) {
        return mq2.e;
    }
}

