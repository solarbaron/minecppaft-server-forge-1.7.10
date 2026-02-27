/*
 * Decompiled with CFR 0.152.
 */
public class ki
extends ft {
    private long a;

    @Override
    public void a(et et2) {
        this.a = et2.readLong();
    }

    @Override
    public void b(et et2) {
        et2.writeLong(this.a);
    }

    public void a(kh kh2) {
        kh2.a(this);
    }

    @Override
    public boolean a() {
        return true;
    }

    public long c() {
        return this.a;
    }
}

