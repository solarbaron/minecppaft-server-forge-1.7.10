/*
 * Decompiled with CFR 0.152.
 */
public class ds {
    public static final ds a = new dt(0L);
    private final long b;
    private long c;

    public ds(long l2) {
        this.b = l2;
    }

    public void a(long l2) {
        this.c += l2 / 8L;
        if (this.c > this.b) {
            throw new RuntimeException("Tried to read NBT tag that was too big; tried to allocate: " + this.c + "bytes where max allowed: " + this.b);
        }
    }
}

