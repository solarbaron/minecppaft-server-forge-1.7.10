/*
 * Decompiled with CFR 0.152.
 */
import java.util.Iterator;
import java.util.Set;
import java.util.TreeMap;

public class agy {
    private TreeMap a = new TreeMap();

    public agy() {
        this.a("doFireTick", "true");
        this.a("mobGriefing", "true");
        this.a("keepInventory", "false");
        this.a("doMobSpawning", "true");
        this.a("doMobLoot", "true");
        this.a("doTileDrops", "true");
        this.a("commandBlockOutput", "true");
        this.a("naturalRegeneration", "true");
        this.a("doDaylightCycle", "true");
    }

    public void a(String string, String string2) {
        this.a.put(string, new agz(string2));
    }

    public void b(String string, String string2) {
        agz agz2 = (agz)this.a.get(string);
        if (agz2 != null) {
            agz2.a(string2);
        } else {
            this.a(string, string2);
        }
    }

    public String a(String string) {
        agz agz2 = (agz)this.a.get(string);
        if (agz2 != null) {
            return agz2.a();
        }
        return "";
    }

    public boolean b(String string) {
        agz agz2 = (agz)this.a.get(string);
        if (agz2 != null) {
            return agz2.b();
        }
        return false;
    }

    public dh a() {
        dh dh2 = new dh();
        for (String string : this.a.keySet()) {
            agz agz2 = (agz)this.a.get(string);
            dh2.a(string, agz2.a());
        }
        return dh2;
    }

    public void a(dh dh2) {
        Set set = dh2.c();
        Iterator iterator = set.iterator();
        while (iterator.hasNext()) {
            String string;
            String string2 = string = (String)iterator.next();
            String string3 = dh2.j(string);
            this.b(string2, string3);
        }
    }

    public String[] b() {
        return this.a.keySet().toArray(new String[0]);
    }

    public boolean e(String string) {
        return this.a.containsKey(string);
    }
}

