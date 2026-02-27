/*
 * Decompiled with CFR 0.152.
 */
import java.util.Collection;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class bad
extends ayl {
    private static final Logger a = LogManager.getLogger();
    private bac b;
    private dh c;

    public bad() {
        this("scoreboard");
    }

    public bad(String string) {
        super(string);
    }

    public void a(bac bac2) {
        this.b = bac2;
        if (this.c != null) {
            this.a(this.c);
        }
    }

    @Override
    public void a(dh dh2) {
        if (this.b == null) {
            this.c = dh2;
            return;
        }
        this.b(dh2.c("Objectives", 10));
        this.c(dh2.c("PlayerScores", 10));
        if (dh2.b("DisplaySlots", 10)) {
            this.c(dh2.m("DisplaySlots"));
        }
        if (dh2.b("Teams", 9)) {
            this.a(dh2.c("Teams", 10));
        }
    }

    protected void a(dq dq2) {
        for (int i2 = 0; i2 < dq2.c(); ++i2) {
            dh dh2 = dq2.b(i2);
            azy azy2 = this.b.f(dh2.j("Name"));
            azy2.a(dh2.j("DisplayName"));
            azy2.b(dh2.j("Prefix"));
            azy2.c(dh2.j("Suffix"));
            if (dh2.b("AllowFriendlyFire", 99)) {
                azy2.a(dh2.n("AllowFriendlyFire"));
            }
            if (dh2.b("SeeFriendlyInvisibles", 99)) {
                azy2.b(dh2.n("SeeFriendlyInvisibles"));
            }
            this.a(azy2, dh2.c("Players", 8));
        }
    }

    protected void a(azy azy2, dq dq2) {
        for (int i2 = 0; i2 < dq2.c(); ++i2) {
            this.b.a(dq2.f(i2), azy2.b());
        }
    }

    protected void c(dh dh2) {
        for (int i2 = 0; i2 < 3; ++i2) {
            if (!dh2.b("slot_" + i2, 8)) continue;
            String string = dh2.j("slot_" + i2);
            azx azx2 = this.b.b(string);
            this.b.a(i2, azx2);
        }
    }

    protected void b(dq dq2) {
        for (int i2 = 0; i2 < dq2.c(); ++i2) {
            dh dh2 = dq2.b(i2);
            bah bah2 = (bah)bah.a.get(dh2.j("CriteriaName"));
            azx azx2 = this.b.a(dh2.j("Name"), bah2);
            azx2.a(dh2.j("DisplayName"));
        }
    }

    protected void c(dq dq2) {
        for (int i2 = 0; i2 < dq2.c(); ++i2) {
            dh dh2 = dq2.b(i2);
            azx azx2 = this.b.b(dh2.j("Objective"));
            azz azz2 = this.b.a(dh2.j("Name"), azx2);
            azz2.c(dh2.f("Score"));
        }
    }

    @Override
    public void b(dh dh2) {
        if (this.b == null) {
            a.warn("Tried to save scoreboard without having a scoreboard...");
            return;
        }
        dh2.a("Objectives", this.b());
        dh2.a("PlayerScores", this.e());
        dh2.a("Teams", this.a());
        this.d(dh2);
    }

    protected dq a() {
        dq dq2 = new dq();
        Collection collection = this.b.g();
        for (azy azy2 : collection) {
            dh dh2 = new dh();
            dh2.a("Name", azy2.b());
            dh2.a("DisplayName", azy2.c());
            dh2.a("Prefix", azy2.e());
            dh2.a("Suffix", azy2.f());
            dh2.a("AllowFriendlyFire", azy2.g());
            dh2.a("SeeFriendlyInvisibles", azy2.h());
            dq dq3 = new dq();
            for (String string : azy2.d()) {
                dq3.a(new dx(string));
            }
            dh2.a("Players", dq3);
            dq2.a(dh2);
        }
        return dq2;
    }

    protected void d(dh dh2) {
        dh dh3 = new dh();
        boolean bl2 = false;
        for (int i2 = 0; i2 < 3; ++i2) {
            azx azx2 = this.b.a(i2);
            if (azx2 == null) continue;
            dh3.a("slot_" + i2, azx2.b());
            bl2 = true;
        }
        if (bl2) {
            dh2.a("DisplaySlots", dh3);
        }
    }

    protected dq b() {
        dq dq2 = new dq();
        Collection collection = this.b.c();
        for (azx azx2 : collection) {
            dh dh2 = new dh();
            dh2.a("Name", azx2.b());
            dh2.a("CriteriaName", azx2.c().a());
            dh2.a("DisplayName", azx2.d());
            dq2.a(dh2);
        }
        return dq2;
    }

    protected dq e() {
        dq dq2 = new dq();
        Collection collection = this.b.e();
        for (azz azz2 : collection) {
            dh dh2 = new dh();
            dh2.a("Name", azz2.e());
            dh2.a("Objective", azz2.d().b());
            dh2.a("Score", azz2.c());
            dq2.a(dh2);
        }
        return dq2;
    }
}

