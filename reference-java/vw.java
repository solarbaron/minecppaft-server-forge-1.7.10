/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.List;

public class vw {
    sw a;
    List b = new ArrayList();
    List c = new ArrayList();

    public vw(sw sw2) {
        this.a = sw2;
    }

    public void a() {
        this.b.clear();
        this.c.clear();
    }

    public boolean a(sa sa2) {
        if (this.b.contains(sa2)) {
            return true;
        }
        if (this.c.contains(sa2)) {
            return false;
        }
        this.a.o.C.a("canSee");
        boolean bl2 = this.a.p(sa2);
        this.a.o.C.b();
        if (bl2) {
            this.b.add(sa2);
        } else {
            this.c.add(sa2);
        }
        return bl2;
    }
}

