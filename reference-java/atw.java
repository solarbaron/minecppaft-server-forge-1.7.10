/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.List;
import java.util.Random;

public class atw
extends atg {
    public att b;
    public List c;
    public List d;
    public ArrayList e = new ArrayList();

    public atw() {
    }

    public atw(Random random, int n2, int n3) {
        super(random, n2, n3);
        this.c = new ArrayList();
        for (att att2 : atf.b()) {
            att2.c = 0;
            this.c.add(att2);
        }
        this.d = new ArrayList();
        for (att att2 : atf.c()) {
            att2.c = 0;
            this.d.add(att2);
        }
    }

    @Override
    protected void b(dh dh2) {
        super.b(dh2);
    }

    @Override
    protected void a(dh dh2) {
        super.a(dh2);
    }
}

