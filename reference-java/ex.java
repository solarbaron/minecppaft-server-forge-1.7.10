/*
 * Decompiled with CFR 0.152.
 */
import java.util.concurrent.atomic.AtomicReference;

class ex {
    private AtomicReference[] a = new AtomicReference[100];

    public ex() {
        for (int i2 = 0; i2 < 100; ++i2) {
            this.a[i2] = new AtomicReference<ey>(new ey(0L, 0, 0.0, null));
        }
    }

    public void a(int n2, long l2) {
        block4: {
            try {
                ey ey2;
                ey ey3;
                if (n2 < 0 || n2 >= 100) {
                    return;
                }
                while (!this.a[n2].compareAndSet(ey3 = (ey)this.a[n2].get(), ey2 = ey3.a(l2))) {
                }
            }
            catch (Exception exception) {
                if (!eu.i().isDebugEnabled()) break block4;
                eu.i().debug(eu.j(), "NetStat failed with packetId: " + n2, (Throwable)exception);
            }
        }
    }

    public long a() {
        long l2 = 0L;
        for (int i2 = 0; i2 < 100; ++i2) {
            l2 += ((ey)this.a[i2].get()).a();
        }
        return l2;
    }

    public long b() {
        long l2 = 0L;
        for (int i2 = 0; i2 < 100; ++i2) {
            l2 += (long)((ey)this.a[i2].get()).b();
        }
        return l2;
    }

    public ew c() {
        int n2 = -1;
        ey ey2 = new ey(-1L, -1, 0.0, null);
        for (int i2 = 0; i2 < 100; ++i2) {
            ey ey3 = (ey)this.a[i2].get();
            if (ey.a(ey3) <= ey.a(ey2)) continue;
            n2 = i2;
            ey2 = ey3;
        }
        return new ew(n2, ey2);
    }

    public ew d() {
        int n2 = -1;
        ey ey2 = new ey(-1L, -1, 0.0, null);
        for (int i2 = 0; i2 < 100; ++i2) {
            ey ey3 = (ey)this.a[i2].get();
            if (ey.b(ey3) <= ey.b(ey2)) continue;
            n2 = i2;
            ey2 = ey3;
        }
        return new ew(n2, ey2);
    }

    public ew a(int n2) {
        if (n2 < 0 || n2 >= 100) {
            return null;
        }
        return new ew(n2, (ey)this.a[n2].get());
    }
}

