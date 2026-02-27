/*
 * Decompiled with CFR 0.152.
 */
import java.util.Iterator;
import java.util.Map;
import org.apache.commons.lang3.StringUtils;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class zu
extends zs {
    private static final Logger f = LogManager.getLogger();
    private rb g = new aaw();
    private rb h = new zv(this, "Repair", true, 2);
    private ahb i;
    private int j;
    private int k;
    private int l;
    public int a;
    private int m;
    private String n;
    private final yz o;

    public zu(yx yx2, ahb ahb2, int n2, int n3, int n4, yz yz2) {
        int n5;
        this.i = ahb2;
        this.j = n2;
        this.k = n3;
        this.l = n4;
        this.o = yz2;
        this.a(new aay(this.h, 0, 27, 47));
        this.a(new aay(this.h, 1, 76, 47));
        this.a(new zw(this, this.g, 2, 134, 47, ahb2, n2, n3, n4));
        for (n5 = 0; n5 < 3; ++n5) {
            for (int i2 = 0; i2 < 9; ++i2) {
                this.a(new aay(yx2, i2 + n5 * 9 + 9, 8 + i2 * 18, 84 + n5 * 18));
            }
        }
        for (n5 = 0; n5 < 9; ++n5) {
            this.a(new aay(yx2, n5, 8 + n5 * 18, 142));
        }
    }

    @Override
    public void a(rb rb2) {
        super.a(rb2);
        if (rb2 == this.h) {
            this.e();
        }
    }

    public void e() {
        int n2;
        int n3;
        int n4;
        int n5;
        add add2 = this.h.a(0);
        this.a = 0;
        int n6 = 0;
        int n7 = 0;
        int n8 = 0;
        if (add2 == null) {
            this.g.a(0, null);
            this.a = 0;
            return;
        }
        add add3 = add2.m();
        add add4 = this.h.a(1);
        Map map = afv.a(add3);
        boolean bl2 = false;
        n7 += add2.C() + (add4 == null ? 0 : add4.C());
        this.m = 0;
        if (add4 != null) {
            boolean bl3 = bl2 = add4.b() == ade.bR && ade.bR.g(add4).c() > 0;
            if (add3.g() && add3.b().a(add2, add4)) {
                int n9;
                n5 = Math.min(add3.j(), add3.l() / 4);
                if (n5 <= 0) {
                    this.g.a(0, null);
                    this.a = 0;
                    return;
                }
                for (n9 = 0; n5 > 0 && n9 < add4.b; ++n9) {
                    n4 = add3.j() - n5;
                    add3.b(n4);
                    n6 += Math.max(1, n5 / 100) + map.size();
                    n5 = Math.min(add3.j(), add3.l() / 4);
                }
                this.m = n9;
            } else {
                if (!(bl2 || add3.b() == add4.b() && add3.g())) {
                    this.g.a(0, null);
                    this.a = 0;
                    return;
                }
                if (add3.g() && !bl2) {
                    int n10 = add2.l() - add2.j();
                    int n11 = add4.l() - add4.j();
                    n4 = n11 + add3.l() * 12 / 100;
                    int n12 = n10 + n4;
                    n3 = add3.l() - n12;
                    if (n3 < 0) {
                        n3 = 0;
                    }
                    if (n3 < add3.k()) {
                        add3.b(n3);
                        n6 += Math.max(1, n4 / 100);
                    }
                }
                Map map2 = afv.a(add4);
                Iterator iterator = map2.keySet().iterator();
                while (iterator.hasNext()) {
                    n4 = (Integer)iterator.next();
                    aft aft2 = aft.b[n4];
                    n3 = map.containsKey(n4) ? (Integer)map.get(n4) : 0;
                    n2 = (Integer)map2.get(n4);
                    n2 = n3 == n2 ? ++n2 : Math.max(n2, n3);
                    int n13 = n2 - n3;
                    boolean bl4 = aft2.a(add2);
                    if (this.o.bE.d || add2.b() == ade.bR) {
                        bl4 = true;
                    }
                    Iterator iterator2 = map.keySet().iterator();
                    while (iterator2.hasNext()) {
                        int n14 = (Integer)iterator2.next();
                        if (n14 == n4 || aft2.a(aft.b[n14])) continue;
                        bl4 = false;
                        n6 += n13;
                    }
                    if (!bl4) continue;
                    if (n2 > aft2.b()) {
                        n2 = aft2.b();
                    }
                    map.put(n4, n2);
                    int n15 = 0;
                    switch (aft2.c()) {
                        case 10: {
                            n15 = 1;
                            break;
                        }
                        case 5: {
                            n15 = 2;
                            break;
                        }
                        case 2: {
                            n15 = 4;
                            break;
                        }
                        case 1: {
                            n15 = 8;
                        }
                    }
                    if (bl2) {
                        n15 = Math.max(1, n15 / 2);
                    }
                    n6 += n15 * n13;
                }
            }
        }
        if (StringUtils.isBlank(this.n)) {
            if (add2.u()) {
                n8 = add2.g() ? 7 : add2.b * 5;
                n6 += n8;
                add3.t();
            }
        } else if (!this.n.equals(add2.s())) {
            n8 = add2.g() ? 7 : add2.b * 5;
            n6 += n8;
            if (add2.u()) {
                n7 += n8 / 2;
            }
            add3.c(this.n);
        }
        n5 = 0;
        Iterator iterator = map.keySet().iterator();
        while (iterator.hasNext()) {
            n4 = (Integer)iterator.next();
            aft aft3 = aft.b[n4];
            n3 = (Integer)map.get(n4);
            n2 = 0;
            ++n5;
            switch (aft3.c()) {
                case 10: {
                    n2 = 1;
                    break;
                }
                case 5: {
                    n2 = 2;
                    break;
                }
                case 2: {
                    n2 = 4;
                    break;
                }
                case 1: {
                    n2 = 8;
                }
            }
            if (bl2) {
                n2 = Math.max(1, n2 / 2);
            }
            n7 += n5 + n3 * n2;
        }
        if (bl2) {
            n7 = Math.max(1, n7 / 2);
        }
        this.a = n7 + n6;
        if (n6 <= 0) {
            add3 = null;
        }
        if (n8 == n6 && n8 > 0 && this.a >= 40) {
            this.a = 39;
        }
        if (this.a >= 40 && !this.o.bE.d) {
            add3 = null;
        }
        if (add3 != null) {
            int n16 = add3.C();
            if (add4 != null && n16 < add4.C()) {
                n16 = add4.C();
            }
            if (add3.u()) {
                n16 -= 9;
            }
            if (n16 < 0) {
                n16 = 0;
            }
            add3.c(n16 += 2);
            afv.a(map, add3);
        }
        this.g.a(0, add3);
        this.b();
    }

    @Override
    public void a(aac aac2) {
        super.a(aac2);
        aac2.a((zs)this, 0, this.a);
    }

    @Override
    public void b(yz yz2) {
        super.b(yz2);
        if (this.i.E) {
            return;
        }
        for (int i2 = 0; i2 < this.h.a(); ++i2) {
            add add2 = this.h.a_(i2);
            if (add2 == null) continue;
            yz2.a(add2, false);
        }
    }

    @Override
    public boolean a(yz yz2) {
        if (this.i.a(this.j, this.k, this.l) != ajn.bQ) {
            return false;
        }
        return !(yz2.e((double)this.j + 0.5, (double)this.k + 0.5, (double)this.l + 0.5) > 64.0);
    }

    @Override
    public add b(yz yz2, int n2) {
        add add2 = null;
        aay aay2 = (aay)this.c.get(n2);
        if (aay2 != null && aay2.e()) {
            add add3 = aay2.d();
            add2 = add3.m();
            if (n2 == 2) {
                if (!this.a(add3, 3, 39, true)) {
                    return null;
                }
                aay2.a(add3, add2);
            } else if (n2 == 0 || n2 == 1 ? !this.a(add3, 3, 39, false) : n2 >= 3 && n2 < 39 && !this.a(add3, 0, 2, false)) {
                return null;
            }
            if (add3.b == 0) {
                aay2.c(null);
            } else {
                aay2.f();
            }
            if (add3.b == add2.b) {
                return null;
            }
            aay2.a(yz2, add3);
        }
        return add2;
    }

    public void a(String string) {
        this.n = string;
        if (this.a(2).e()) {
            add add2 = this.a(2).d();
            if (StringUtils.isBlank(string)) {
                add2.t();
            } else {
                add2.c(this.n);
            }
        }
        this.e();
    }

    static /* synthetic */ rb a(zu zu2) {
        return zu2.h;
    }

    static /* synthetic */ int b(zu zu2) {
        return zu2.m;
    }
}

