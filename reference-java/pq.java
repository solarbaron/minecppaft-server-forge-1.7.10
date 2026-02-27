/*
 * Decompiled with CFR 0.152.
 */
import com.google.common.collect.Maps;
import java.util.Map;

public class pq {
    protected final Map a = Maps.newConcurrentMap();

    public boolean a(pb pb2) {
        return this.a((ph)pb2) > 0;
    }

    public boolean b(pb pb2) {
        return pb2.c == null || this.a(pb2.c);
    }

    public void b(yz yz2, ph ph2, int n2) {
        if (ph2.d() && !this.b((pb)ph2)) {
            return;
        }
        this.a(yz2, ph2, this.a(ph2) + n2);
    }

    public void a(yz yz2, ph ph2, int n2) {
        pn pn2 = (pn)this.a.get(ph2);
        if (pn2 == null) {
            pn2 = new pn();
            this.a.put(ph2, pn2);
        }
        pn2.a(n2);
    }

    public int a(ph ph2) {
        pn pn2 = (pn)this.a.get(ph2);
        return pn2 == null ? 0 : pn2.a();
    }

    public po b(ph ph2) {
        pn pn2 = (pn)this.a.get(ph2);
        if (pn2 != null) {
            return pn2.b();
        }
        return null;
    }

    public po a(ph ph2, po po2) {
        pn pn2 = (pn)this.a.get(ph2);
        if (pn2 == null) {
            pn2 = new pn();
            this.a.put(ph2, pn2);
        }
        pn2.a(po2);
        return po2;
    }
}

