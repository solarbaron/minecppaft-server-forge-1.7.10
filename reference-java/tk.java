/*
 * Decompiled with CFR 0.152.
 */
public abstract class tk
implements th {
    private final String a;
    private final double b;
    private boolean c;

    protected tk(String string, double d2) {
        this.a = string;
        this.b = d2;
        if (string == null) {
            throw new IllegalArgumentException("Name cannot be null!");
        }
    }

    @Override
    public String a() {
        return this.a;
    }

    @Override
    public double b() {
        return this.b;
    }

    @Override
    public boolean c() {
        return this.c;
    }

    public tk a(boolean bl2) {
        this.c = bl2;
        return this;
    }

    public int hashCode() {
        return this.a.hashCode();
    }
}

