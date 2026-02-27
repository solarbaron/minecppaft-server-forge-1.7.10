/*
 * Decompiled with CFR 0.152.
 */
public enum se {
    a,
    b,
    c,
    d,
    e,
    f;


    public int a(double d2) {
        double d3 = d2 - ((double)qh.c(d2) + 0.5);
        switch (this) {
            case a: {
                if (d3 < 0.0 ? d3 < -0.3125 : d3 < 0.3125) {
                    return qh.f(d2 * 32.0);
                }
                return qh.c(d2 * 32.0);
            }
            case b: {
                if (d3 < 0.0 ? d3 < -0.3125 : d3 < 0.3125) {
                    return qh.c(d2 * 32.0);
                }
                return qh.f(d2 * 32.0);
            }
            case c: {
                if (d3 > 0.0) {
                    return qh.c(d2 * 32.0);
                }
                return qh.f(d2 * 32.0);
            }
            case d: {
                if (d3 < 0.0 ? d3 < -0.1875 : d3 < 0.1875) {
                    return qh.f(d2 * 32.0);
                }
                return qh.c(d2 * 32.0);
            }
            case e: {
                if (d3 < 0.0 ? d3 < -0.1875 : d3 < 0.1875) {
                    return qh.c(d2 * 32.0);
                }
                return qh.f(d2 * 32.0);
            }
        }
        if (d3 > 0.0) {
            return qh.f(d2 * 32.0);
        }
        return qh.c(d2 * 32.0);
    }
}

