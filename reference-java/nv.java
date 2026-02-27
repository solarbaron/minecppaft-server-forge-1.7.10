/*
 * Decompiled with CFR 0.152.
 */
import com.mojang.authlib.GameProfile;
import java.util.Date;

class nv {
    private final GameProfile b;
    private final Date c;
    final /* synthetic */ ns a;

    private nv(ns ns2, GameProfile gameProfile, Date date) {
        this.a = ns2;
        this.b = gameProfile;
        this.c = date;
    }

    public GameProfile a() {
        return this.b;
    }

    public Date b() {
        return this.c;
    }

    /* synthetic */ nv(ns ns2, GameProfile gameProfile, Date date, nt nt2) {
        this(ns2, gameProfile, date);
    }

    static /* synthetic */ Date a(nv nv2) {
        return nv2.c;
    }
}

