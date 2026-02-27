/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import net.minecraft.server.MinecraftServer;

public class lp
extends bac {
    private final MinecraftServer a;
    private final Set b = new HashSet();
    private bad c;

    public lp(MinecraftServer minecraftServer) {
        this.a = minecraftServer;
    }

    @Override
    public void a(azz azz2) {
        super.a(azz2);
        if (this.b.contains(azz2.d())) {
            this.a.ah().a(new ie(azz2, 0));
        }
        this.b();
    }

    @Override
    public void a(String string) {
        super.a(string);
        this.a.ah().a(new ie(string));
        this.b();
    }

    @Override
    public void a(int n2, azx azx2) {
        azx azx3 = this.a(n2);
        super.a(n2, azx2);
        if (azx3 != azx2 && azx3 != null) {
            if (this.h(azx3) > 0) {
                this.a.ah().a(new hv(n2, azx2));
            } else {
                this.g(azx3);
            }
        }
        if (azx2 != null) {
            if (this.b.contains(azx2)) {
                this.a.ah().a(new hv(n2, azx2));
            } else {
                this.e(azx2);
            }
        }
        this.b();
    }

    @Override
    public boolean a(String string, String string2) {
        if (super.a(string, string2)) {
            azy azy2 = this.e(string2);
            this.a.ah().a(new id(azy2, Arrays.asList(string), 3));
            this.b();
            return true;
        }
        return false;
    }

    @Override
    public void a(String string, azy azy2) {
        super.a(string, azy2);
        this.a.ah().a(new id(azy2, Arrays.asList(string), 4));
        this.b();
    }

    @Override
    public void a(azx azx2) {
        super.a(azx2);
        this.b();
    }

    @Override
    public void b(azx azx2) {
        super.b(azx2);
        if (this.b.contains(azx2)) {
            this.a.ah().a(new ic(azx2, 2));
        }
        this.b();
    }

    @Override
    public void c(azx azx2) {
        super.c(azx2);
        if (this.b.contains(azx2)) {
            this.g(azx2);
        }
        this.b();
    }

    @Override
    public void a(azy azy2) {
        super.a(azy2);
        this.a.ah().a(new id(azy2, 0));
        this.b();
    }

    @Override
    public void b(azy azy2) {
        super.b(azy2);
        this.a.ah().a(new id(azy2, 2));
        this.b();
    }

    @Override
    public void c(azy azy2) {
        super.c(azy2);
        this.a.ah().a(new id(azy2, 1));
        this.b();
    }

    public void a(bad bad2) {
        this.c = bad2;
    }

    protected void b() {
        if (this.c != null) {
            this.c.c();
        }
    }

    public List d(azx azx2) {
        ArrayList<ft> arrayList = new ArrayList<ft>();
        arrayList.add(new ic(azx2, 0));
        for (int i2 = 0; i2 < 3; ++i2) {
            if (this.a(i2) != azx2) continue;
            arrayList.add(new hv(i2, azx2));
        }
        for (azz azz2 : this.i(azx2)) {
            arrayList.add(new ie(azz2, 0));
        }
        return arrayList;
    }

    public void e(azx azx2) {
        List list = this.d(azx2);
        for (mw mw2 : this.a.ah().e) {
            for (ft ft2 : list) {
                mw2.a.a(ft2);
            }
        }
        this.b.add(azx2);
    }

    public List f(azx azx2) {
        ArrayList<ft> arrayList = new ArrayList<ft>();
        arrayList.add(new ic(azx2, 1));
        for (int i2 = 0; i2 < 3; ++i2) {
            if (this.a(i2) != azx2) continue;
            arrayList.add(new hv(i2, azx2));
        }
        return arrayList;
    }

    public void g(azx azx2) {
        List list = this.f(azx2);
        for (mw mw2 : this.a.ah().e) {
            for (ft ft2 : list) {
                mw2.a.a(ft2);
            }
        }
        this.b.remove(azx2);
    }

    public int h(azx azx2) {
        int n2 = 0;
        for (int i2 = 0; i2 < 3; ++i2) {
            if (this.a(i2) != azx2) continue;
            ++n2;
        }
        return n2;
    }
}

