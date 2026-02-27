/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

public class adp
extends adb {
    private HashMap a = new HashMap();
    private static final Map b = new LinkedHashMap();

    public adp() {
        this.e(1);
        this.a(true);
        this.f(0);
        this.a(abt.k);
    }

    public List g(add add2) {
        if (!add2.p() || !add2.q().b("CustomPotionEffects", 9)) {
            List list = (List)this.a.get(add2.k());
            if (list == null) {
                list = aen.b(add2.k(), false);
                this.a.put(add2.k(), list);
            }
            return list;
        }
        ArrayList<rw> arrayList = new ArrayList<rw>();
        dq dq2 = add2.q().c("CustomPotionEffects", 10);
        for (int i2 = 0; i2 < dq2.c(); ++i2) {
            dh dh2 = dq2.b(i2);
            rw rw2 = rw.b(dh2);
            if (rw2 == null) continue;
            arrayList.add(rw2);
        }
        return arrayList;
    }

    public List c(int n2) {
        List list = (List)this.a.get(n2);
        if (list == null) {
            list = aen.b(n2, false);
            this.a.put(n2, list);
        }
        return list;
    }

    @Override
    public add b(add add2, ahb ahb2, yz yz2) {
        List list;
        if (!yz2.bE.d) {
            --add2.b;
        }
        if (!ahb2.E && (list = this.g(add2)) != null) {
            for (rw rw2 : list) {
                yz2.c(new rw(rw2));
            }
        }
        if (!yz2.bE.d) {
            if (add2.b <= 0) {
                return new add(ade.bo);
            }
            yz2.bm.a(new add(ade.bo));
        }
        return add2;
    }

    @Override
    public int d_(add add2) {
        return 32;
    }

    @Override
    public aei d(add add2) {
        return aei.c;
    }

    @Override
    public add a(add add2, ahb ahb2, yz yz2) {
        if (adp.g(add2.k())) {
            if (!yz2.bE.d) {
                --add2.b;
            }
            ahb2.a((sa)yz2, "random.bow", 0.5f, 0.4f / (g.nextFloat() * 0.4f + 0.8f));
            if (!ahb2.E) {
                ahb2.d(new zo(ahb2, (sv)yz2, add2));
            }
            return add2;
        }
        yz2.a(add2, this.d_(add2));
        return add2;
    }

    @Override
    public boolean a(add add2, yz yz2, ahb ahb2, int n2, int n3, int n4, int n5, float f2, float f3, float f4) {
        return false;
    }

    public static boolean g(int n2) {
        return (n2 & 0x4000) != 0;
    }

    @Override
    public String n(add add2) {
        List list;
        if (add2.k() == 0) {
            return dd.a("item.emptyPotion.name").trim();
        }
        String string = "";
        if (adp.g(add2.k())) {
            string = dd.a("potion.prefix.grenade").trim() + " ";
        }
        if ((list = ade.bn.g(add2)) != null && !list.isEmpty()) {
            String string2 = ((rw)list.get(0)).f();
            string2 = string2 + ".postfix";
            return string + dd.a(string2).trim();
        }
        String string3 = aen.c(add2.k());
        return dd.a(string3).trim() + " " + super.n(add2);
    }
}

