/*
 * Decompiled with CFR 0.152.
 */
public class to
extends tk {
    private final double a;
    private final double b;
    private String c;

    public to(String string, double d2, double d3, double d4) {
        super(string, d2);
        this.a = d3;
        this.b = d4;
        if (d3 > d4) {
            throw new IllegalArgumentException("Minimum value cannot be bigger than maximum value!");
        }
        if (d2 < d3) {
            throw new IllegalArgumentException("Default value cannot be lower than minimum value!");
        }
        if (d2 > d4) {
            throw new IllegalArgumentException("Default value cannot be bigger than maximum value!");
        }
    }

    public to a(String string) {
        this.c = string;
        return this;
    }

    public String f() {
        return this.c;
    }

    @Override
    public double a(double d2) {
        if (d2 < this.a) {
            d2 = this.a;
        }
        if (d2 > this.b) {
            d2 = this.b;
        }
        return d2;
    }
}

