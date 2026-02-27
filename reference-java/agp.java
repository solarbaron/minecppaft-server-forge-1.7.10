/*
 * Decompiled with CFR 0.152.
 */
import java.text.SimpleDateFormat;
import java.util.Date;
import net.minecraft.server.MinecraftServer;

public abstract class agp
implements ac {
    private static final SimpleDateFormat a = new SimpleDateFormat("HH:mm:ss");
    private int b;
    private boolean c = true;
    private fj d = null;
    private String e = "";
    private String f = "@";

    public int g() {
        return this.b;
    }

    public fj h() {
        return this.d;
    }

    public void a(dh dh2) {
        dh2.a("Command", this.e);
        dh2.a("SuccessCount", this.b);
        dh2.a("CustomName", this.f);
        if (this.d != null) {
            dh2.a("LastOutput", fk.a(this.d));
        }
        dh2.a("TrackOutput", this.c);
    }

    public void b(dh dh2) {
        this.e = dh2.j("Command");
        this.b = dh2.f("SuccessCount");
        if (dh2.b("CustomName", 8)) {
            this.f = dh2.j("CustomName");
        }
        if (dh2.b("LastOutput", 8)) {
            this.d = fk.a(dh2.j("LastOutput"));
        }
        if (dh2.b("TrackOutput", 1)) {
            this.c = dh2.n("TrackOutput");
        }
    }

    @Override
    public boolean a(int n2, String string) {
        return n2 <= 2;
    }

    public void a(String string) {
        this.e = string;
    }

    public String i() {
        return this.e;
    }

    public void a(ahb ahb2) {
        MinecraftServer minecraftServer;
        if (ahb2.E) {
            this.b = 0;
        }
        if ((minecraftServer = MinecraftServer.I()) != null && minecraftServer.ad()) {
            ab ab2 = minecraftServer.J();
            this.b = ab2.a(this, this.e);
        } else {
            this.b = 0;
        }
    }

    @Override
    public String b_() {
        return this.f;
    }

    @Override
    public fj c_() {
        return new fq(this.b_());
    }

    public void b(String string) {
        this.f = string;
    }

    @Override
    public void a(fj fj2) {
        if (this.c && this.d() != null && !this.d().E) {
            this.d = new fq("[" + a.format(new Date()) + "] ").a(fj2);
            this.e();
        }
    }

    public abstract void e();

    public void b(fj fj2) {
        this.d = fj2;
    }
}

