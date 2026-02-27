/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class bac {
    private final Map a = new HashMap();
    private final Map b = new HashMap();
    private final Map c = new HashMap();
    private final azx[] d = new azx[3];
    private final Map e = new HashMap();
    private final Map f = new HashMap();

    public azx b(String string) {
        return (azx)this.a.get(string);
    }

    public azx a(String string, bah bah2) {
        azx azx2 = this.b(string);
        if (azx2 != null) {
            throw new IllegalArgumentException("An objective with the name '" + string + "' already exists!");
        }
        azx2 = new azx(this, string, bah2);
        ArrayList<azx> arrayList = (ArrayList<azx>)this.b.get(bah2);
        if (arrayList == null) {
            arrayList = new ArrayList<azx>();
            this.b.put(bah2, arrayList);
        }
        arrayList.add(azx2);
        this.a.put(string, azx2);
        this.a(azx2);
        return azx2;
    }

    public Collection a(bah bah2) {
        Collection collection = (Collection)this.b.get(bah2);
        return collection == null ? new ArrayList() : new ArrayList(collection);
    }

    public azz a(String string, azx azx2) {
        azz azz2;
        HashMap<azx, azz> hashMap = (HashMap<azx, azz>)this.c.get(string);
        if (hashMap == null) {
            hashMap = new HashMap<azx, azz>();
            this.c.put(string, hashMap);
        }
        if ((azz2 = (azz)hashMap.get(azx2)) == null) {
            azz2 = new azz(this, azx2, string);
            hashMap.put(azx2, azz2);
        }
        return azz2;
    }

    public Collection i(azx azx2) {
        ArrayList<azz> arrayList = new ArrayList<azz>();
        for (Map map : this.c.values()) {
            azz azz2 = (azz)map.get(azx2);
            if (azz2 == null) continue;
            arrayList.add(azz2);
        }
        Collections.sort(arrayList, azz.a);
        return arrayList;
    }

    public Collection c() {
        return this.a.values();
    }

    public Collection d() {
        return this.c.keySet();
    }

    public void c(String string) {
        Map map = (Map)this.c.remove(string);
        if (map != null) {
            this.a(string);
        }
    }

    public Collection e() {
        Collection collection = this.c.values();
        ArrayList arrayList = new ArrayList();
        for (Map map : collection) {
            arrayList.addAll(map.values());
        }
        return arrayList;
    }

    public Map d(String string) {
        HashMap hashMap = (HashMap)this.c.get(string);
        if (hashMap == null) {
            hashMap = new HashMap();
        }
        return hashMap;
    }

    public void k(azx azx2) {
        this.a.remove(azx2.b());
        for (int i2 = 0; i2 < 3; ++i2) {
            if (this.a(i2) != azx2) continue;
            this.a(i2, null);
        }
        List list = (List)this.b.get(azx2.c());
        if (list != null) {
            list.remove(azx2);
        }
        for (Map map : this.c.values()) {
            map.remove(azx2);
        }
        this.c(azx2);
    }

    public void a(int n2, azx azx2) {
        this.d[n2] = azx2;
    }

    public azx a(int n2) {
        return this.d[n2];
    }

    public azy e(String string) {
        return (azy)this.e.get(string);
    }

    public azy f(String string) {
        azy azy2 = this.e(string);
        if (azy2 != null) {
            throw new IllegalArgumentException("A team with the name '" + string + "' already exists!");
        }
        azy2 = new azy(this, string);
        this.e.put(string, azy2);
        this.a(azy2);
        return azy2;
    }

    public void d(azy azy2) {
        this.e.remove(azy2.b());
        for (String string : azy2.d()) {
            this.f.remove(string);
        }
        this.c(azy2);
    }

    public boolean a(String string, String string2) {
        if (!this.e.containsKey(string2)) {
            return false;
        }
        azy azy2 = this.e(string2);
        if (this.i(string) != null) {
            this.g(string);
        }
        this.f.put(string, azy2);
        azy2.d().add(string);
        return true;
    }

    public boolean g(String string) {
        azy azy2 = this.i(string);
        if (azy2 != null) {
            this.a(string, azy2);
            return true;
        }
        return false;
    }

    public void a(String string, azy azy2) {
        if (this.i(string) != azy2) {
            throw new IllegalStateException("Player is either on another team or not on any team. Cannot remove from team '" + azy2.b() + "'.");
        }
        this.f.remove(string);
        azy2.d().remove(string);
    }

    public Collection f() {
        return this.e.keySet();
    }

    public Collection g() {
        return this.e.values();
    }

    public azy i(String string) {
        return (azy)this.f.get(string);
    }

    public void a(azx azx2) {
    }

    public void b(azx azx2) {
    }

    public void c(azx azx2) {
    }

    public void a(azz azz2) {
    }

    public void a(String string) {
    }

    public void a(azy azy2) {
    }

    public void b(azy azy2) {
    }

    public void c(azy azy2) {
    }

    public static String b(int n2) {
        switch (n2) {
            case 0: {
                return "list";
            }
            case 1: {
                return "sidebar";
            }
            case 2: {
                return "belowName";
            }
        }
        return null;
    }

    public static int j(String string) {
        if (string.equalsIgnoreCase("list")) {
            return 0;
        }
        if (string.equalsIgnoreCase("sidebar")) {
            return 1;
        }
        if (string.equalsIgnoreCase("belowName")) {
            return 2;
        }
        return -1;
    }
}

