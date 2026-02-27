/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.List;

public class atd
extends ave {
    private List e = new ArrayList();

    public atd() {
        this.e.add(new ahx(xx.class, 10, 2, 3));
        this.e.add(new ahx(yh.class, 5, 4, 4));
        this.e.add(new ahx(yl.class, 10, 4, 4));
        this.e.add(new ahx(yf.class, 3, 4, 4));
    }

    @Override
    public String a() {
        return "Fortress";
    }

    public List b() {
        return this.e;
    }

    @Override
    protected boolean a(int n2, int n3) {
        int n4 = n2 >> 4;
        int n5 = n3 >> 4;
        this.b.setSeed((long)(n4 ^ n5 << 4) ^ this.c.H());
        this.b.nextInt();
        if (this.b.nextInt(3) != 0) {
            return false;
        }
        if (n2 != (n4 << 4) + 4 + this.b.nextInt(8)) {
            return false;
        }
        return n3 == (n5 << 4) + 4 + this.b.nextInt(8);
    }

    @Override
    protected avm b(int n2, int n3) {
        return new ate(this.c, this.b, n2, n3);
    }
}

