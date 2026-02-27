/*
 * Decompiled with CFR 0.152.
 */
public class ih
extends ft {
    private long a;
    private long b;

    public ih() {
    }

    public ih(long l2, long l3, boolean bl2) {
        this.a = l2;
        this.b = l3;
        if (!bl2) {
            this.b = -this.b;
            if (this.b == 0L) {
                this.b = -1L;
            }
        }
    }

    @Override
    public void a(et et2) {
        this.a = et2.readLong();
        this.b = et2.readLong();
    }

    @Override
    public void b(et et2) {
        et2.writeLong(this.a);
        et2.writeLong(this.b);
    }

    public void a(fv fv2) {
        fv2.a(this);
    }

    @Override
    public String b() {
        return String.format("time=%d,dtime=%d", this.a, this.b);
    }
}

