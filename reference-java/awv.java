/*
 * Decompiled with CFR 0.152.
 */
public class awv {
    public static final awv[] a = new awv[64];
    public static final awv b = new awv(0, 0);
    public static final awv c = new awv(1, 8368696);
    public static final awv d = new awv(2, 16247203);
    public static final awv e = new awv(3, 0xA7A7A7);
    public static final awv f = new awv(4, 0xFF0000);
    public static final awv g = new awv(5, 0xA0A0FF);
    public static final awv h = new awv(6, 0xA7A7A7);
    public static final awv i = new awv(7, 31744);
    public static final awv j = new awv(8, 0xFFFFFF);
    public static final awv k = new awv(9, 10791096);
    public static final awv l = new awv(10, 12020271);
    public static final awv m = new awv(11, 0x707070);
    public static final awv n = new awv(12, 0x4040FF);
    public static final awv o = new awv(13, 6837042);
    public static final awv p = new awv(14, 0xFFFCF5);
    public static final awv q = new awv(15, 14188339);
    public static final awv r = new awv(16, 11685080);
    public static final awv s = new awv(17, 6724056);
    public static final awv t = new awv(18, 0xE5E533);
    public static final awv u = new awv(19, 8375321);
    public static final awv v = new awv(20, 15892389);
    public static final awv w = new awv(21, 0x4C4C4C);
    public static final awv x = new awv(22, 0x999999);
    public static final awv y = new awv(23, 5013401);
    public static final awv z = new awv(24, 8339378);
    public static final awv A = new awv(25, 3361970);
    public static final awv B = new awv(26, 6704179);
    public static final awv C = new awv(27, 6717235);
    public static final awv D = new awv(28, 0x993333);
    public static final awv E = new awv(29, 0x191919);
    public static final awv F = new awv(30, 16445005);
    public static final awv G = new awv(31, 6085589);
    public static final awv H = new awv(32, 4882687);
    public static final awv I = new awv(33, 55610);
    public static final awv J = new awv(34, 1381407);
    public static final awv K = new awv(35, 0x700200);
    public final int L;
    public final int M;

    private awv(int n2, int n3) {
        if (n2 < 0 || n2 > 63) {
            throw new IndexOutOfBoundsException("Map colour ID must be between 0 and 63 (inclusive)");
        }
        this.M = n2;
        this.L = n3;
        awv.a[n2] = this;
    }

    public static awv a(int n2) {
        switch (aka.c(n2)) {
            case 0: {
                return E;
            }
            case 1: {
                return D;
            }
            case 2: {
                return C;
            }
            case 3: {
                return B;
            }
            case 4: {
                return A;
            }
            case 5: {
                return z;
            }
            case 6: {
                return y;
            }
            case 7: {
                return x;
            }
            case 8: {
                return w;
            }
            case 9: {
                return v;
            }
            case 10: {
                return u;
            }
            case 11: {
                return t;
            }
            case 12: {
                return s;
            }
            case 13: {
                return r;
            }
            case 14: {
                return q;
            }
            case 15: {
                return j;
            }
        }
        return b;
    }
}

