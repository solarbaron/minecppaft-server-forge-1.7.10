/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Set;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class mn {
    private static final Logger a = LogManager.getLogger();
    private final mt b;
    private Set c = new HashSet();
    private pz d = new pz();
    private int e;

    public mn(mt mt2) {
        this.b = mt2;
        this.e = mt2.q().ah().d();
    }

    public void a(sa sa2) {
        if (sa2 instanceof mw) {
            this.a(sa2, 512, 2);
            mw mw2 = (mw)sa2;
            for (my my2 : this.c) {
                if (my2.a == mw2) continue;
                my2.b(mw2);
            }
        } else if (sa2 instanceof xe) {
            this.a(sa2, 64, 5, true);
        } else if (sa2 instanceof zc) {
            this.a(sa2, 64, 20, false);
        } else if (sa2 instanceof zi) {
            this.a(sa2, 64, 10, false);
        } else if (sa2 instanceof ze) {
            this.a(sa2, 64, 10, false);
        } else if (sa2 instanceof zj) {
            this.a(sa2, 64, 10, true);
        } else if (sa2 instanceof zm) {
            this.a(sa2, 64, 10, true);
        } else if (sa2 instanceof zd) {
            this.a(sa2, 64, 4, true);
        } else if (sa2 instanceof zl) {
            this.a(sa2, 64, 10, true);
        } else if (sa2 instanceof zo) {
            this.a(sa2, 64, 10, true);
        } else if (sa2 instanceof zn) {
            this.a(sa2, 64, 10, true);
        } else if (sa2 instanceof zf) {
            this.a(sa2, 64, 10, true);
        } else if (sa2 instanceof xk) {
            this.a(sa2, 64, 20, true);
        } else if (sa2 instanceof xl) {
            this.a(sa2, 80, 3, true);
        } else if (sa2 instanceof xi) {
            this.a(sa2, 80, 3, true);
        } else if (sa2 instanceof ws) {
            this.a(sa2, 64, 3, true);
        } else if (sa2 instanceof xc) {
            this.a(sa2, 80, 3, false);
        } else if (sa2 instanceof we) {
            this.a(sa2, 80, 3, false);
        } else if (sa2 instanceof ry) {
            this.a(sa2, 80, 3, true);
        } else if (sa2 instanceof xa) {
            this.a(sa2, 160, 3, true);
        } else if (sa2 instanceof xw) {
            this.a(sa2, 160, 10, true);
        } else if (sa2 instanceof xj) {
            this.a(sa2, 160, 20, true);
        } else if (sa2 instanceof ss) {
            this.a(sa2, 160, Integer.MAX_VALUE, false);
        } else if (sa2 instanceof sq) {
            this.a(sa2, 160, 20, true);
        } else if (sa2 instanceof wz) {
            this.a(sa2, 256, Integer.MAX_VALUE, false);
        }
    }

    public void a(sa sa2, int n2, int n3) {
        this.a(sa2, n2, n3, false);
    }

    public void a(sa sa2, int n2, int n3, boolean bl2) {
        if (n2 > this.e) {
            n2 = this.e;
        }
        try {
            if (this.d.b(sa2.y())) {
                throw new IllegalStateException("Entity is already tracked!");
            }
            my my2 = new my(sa2, n2, n3, bl2);
            this.c.add(my2);
            this.d.a(sa2.y(), my2);
            my2.b(this.b.h);
        }
        catch (Throwable throwable) {
            b b2 = b.a(throwable, "Adding entity to track");
            k k2 = b2.a("Entity To Track");
            k2.a("Tracking range", n2 + " blocks");
            k2.a("Update interval", new mo(this, n3));
            sa2.a(k2);
            k k3 = b2.a("Entity That Is Already Tracked");
            ((my)this.d.a((int)sa2.y())).a.a(k3);
            try {
                throw new s(b2);
            }
            catch (s s2) {
                a.error("\"Silently\" catching entity tracking error.", (Throwable)s2);
            }
        }
    }

    public void b(sa sa2) {
        Object object;
        if (sa2 instanceof mw) {
            object = (mw)sa2;
            for (my my2 : this.c) {
                my2.a((mw)object);
            }
        }
        if ((object = (my)this.d.d(sa2.y())) != null) {
            this.c.remove(object);
            ((my)object).a();
        }
    }

    public void a() {
        ArrayList<mw> arrayList = new ArrayList<mw>();
        for (Object object : this.c) {
            ((my)object).a(this.b.h);
            if (!((my)object).n || !(((my)object).a instanceof mw)) continue;
            arrayList.add((mw)((my)object).a);
        }
        for (int i2 = 0; i2 < arrayList.size(); ++i2) {
            Object object;
            object = (mw)arrayList.get(i2);
            for (my my2 : this.c) {
                if (my2.a == object) continue;
                my2.b((mw)object);
            }
        }
    }

    public void a(sa sa2, ft ft2) {
        my my2 = (my)this.d.a(sa2.y());
        if (my2 != null) {
            my2.a(ft2);
        }
    }

    public void b(sa sa2, ft ft2) {
        my my2 = (my)this.d.a(sa2.y());
        if (my2 != null) {
            my2.b(ft2);
        }
    }

    public void a(mw mw2) {
        for (my my2 : this.c) {
            my2.c(mw2);
        }
    }

    public void a(mw mw2, apx apx2) {
        for (my my2 : this.c) {
            if (my2.a == mw2 || my2.a.ah != apx2.g || my2.a.aj != apx2.h) continue;
            my2.b(mw2);
        }
    }
}

