/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class uj {
    private static final Logger a = LogManager.getLogger();
    private List b = new ArrayList();
    private List c = new ArrayList();
    private final qi d;
    private int e;
    private int f = 3;

    public uj(qi qi2) {
        this.d = qi2;
    }

    public void a(int n2, ui ui2) {
        this.b.add(new uk(this, n2, ui2));
    }

    public void a(ui ui2) {
        Iterator iterator = this.b.iterator();
        while (iterator.hasNext()) {
            uk uk2 = (uk)iterator.next();
            ui ui3 = uk2.a;
            if (ui3 != ui2) continue;
            if (this.c.contains(uk2)) {
                ui3.d();
                this.c.remove(uk2);
            }
            iterator.remove();
        }
    }

    public void a() {
        ArrayList<uk> arrayList = new ArrayList<uk>();
        if (this.e++ % this.f == 0) {
            for (uk uk2 : this.b) {
                boolean bl2 = this.c.contains(uk2);
                if (bl2) {
                    if (this.b(uk2) && this.a(uk2)) continue;
                    uk2.a.d();
                    this.c.remove(uk2);
                }
                if (!this.b(uk2) || !uk2.a.a()) continue;
                arrayList.add(uk2);
                this.c.add(uk2);
            }
        } else {
            Iterator iterator = this.c.iterator();
            while (iterator.hasNext()) {
                uk uk2;
                uk2 = (uk)iterator.next();
                if (uk2.a.b()) continue;
                uk2.a.d();
                iterator.remove();
            }
        }
        this.d.a("goalStart");
        for (uk uk2 : arrayList) {
            this.d.a(uk2.a.getClass().getSimpleName());
            uk2.a.c();
            this.d.b();
        }
        this.d.b();
        this.d.a("goalTick");
        for (uk uk2 : this.c) {
            uk2.a.e();
        }
        this.d.b();
    }

    private boolean a(uk uk2) {
        this.d.a("canContinue");
        boolean bl2 = uk2.a.b();
        this.d.b();
        return bl2;
    }

    private boolean b(uk uk2) {
        this.d.a("canUse");
        for (uk uk3 : this.b) {
            if (uk3 == uk2) continue;
            if (uk2.b >= uk3.b) {
                if (!this.c.contains(uk3) || this.a(uk2, uk3)) continue;
                this.d.b();
                return false;
            }
            if (!this.c.contains(uk3) || uk3.a.i()) continue;
            this.d.b();
            return false;
        }
        this.d.b();
        return true;
    }

    private boolean a(uk uk2, uk uk3) {
        return (uk2.a.j() & uk3.a.j()) == 0;
    }
}

