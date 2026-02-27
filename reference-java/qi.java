/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class qi {
    private static final Logger b = LogManager.getLogger();
    private final List c = new ArrayList();
    private final List d = new ArrayList();
    public boolean a;
    private String e = "";
    private final Map f = new HashMap();

    public void a() {
        this.f.clear();
        this.e = "";
        this.c.clear();
    }

    public void a(String string) {
        if (!this.a) {
            return;
        }
        if (this.e.length() > 0) {
            this.e = this.e + ".";
        }
        this.e = this.e + string;
        this.c.add(this.e);
        this.d.add(System.nanoTime());
    }

    public void b() {
        if (!this.a) {
            return;
        }
        long l2 = System.nanoTime();
        long l3 = (Long)this.d.remove(this.d.size() - 1);
        this.c.remove(this.c.size() - 1);
        long l4 = l2 - l3;
        if (this.f.containsKey(this.e)) {
            this.f.put(this.e, (Long)this.f.get(this.e) + l4);
        } else {
            this.f.put(this.e, l4);
        }
        if (l4 > 100000000L) {
            b.warn("Something's taking too long! '" + this.e + "' took aprox " + (double)l4 / 1000000.0 + " ms");
        }
        this.e = !this.c.isEmpty() ? (String)this.c.get(this.c.size() - 1) : "";
    }

    public List b(String string) {
        if (!this.a) {
            return null;
        }
        String string2 = string;
        long l2 = this.f.containsKey("root") ? (Long)this.f.get("root") : 0L;
        long l3 = this.f.containsKey(string) ? (Long)this.f.get(string) : -1L;
        ArrayList<qj> arrayList = new ArrayList<qj>();
        if (string.length() > 0) {
            string = string + ".";
        }
        long l4 = 0L;
        for (Object object : this.f.keySet()) {
            if (((String)object).length() <= string.length() || !((String)object).startsWith(string) || ((String)object).indexOf(".", string.length() + 1) >= 0) continue;
            l4 += ((Long)this.f.get(object)).longValue();
        }
        float f2 = l4;
        if (l4 < l3) {
            l4 = l3;
        }
        if (l2 < l4) {
            l2 = l4;
        }
        for (String string3 : this.f.keySet()) {
            if (string3.length() <= string.length() || !string3.startsWith(string) || string3.indexOf(".", string.length() + 1) >= 0) continue;
            long l5 = (Long)this.f.get(string3);
            double d2 = (double)l5 * 100.0 / (double)l4;
            double d3 = (double)l5 * 100.0 / (double)l2;
            String string4 = string3.substring(string.length());
            arrayList.add(new qj(string4, d2, d3));
        }
        for (String string3 : this.f.keySet()) {
            this.f.put(string3, (Long)this.f.get(string3) * 999L / 1000L);
        }
        if ((float)l4 > f2) {
            arrayList.add(new qj("unspecified", (double)((float)l4 - f2) * 100.0 / (double)l4, (double)((float)l4 - f2) * 100.0 / (double)l2));
        }
        Collections.sort(arrayList);
        arrayList.add(0, new qj(string2, 100.0, (double)l4 * 100.0 / (double)l2));
        return arrayList;
    }

    public void c(String string) {
        this.b();
        this.a(string);
    }

    public String c() {
        return this.c.size() == 0 ? "[UNKNOWN]" : (String)this.c.get(this.c.size() - 1);
    }
}

