/*
 * Decompiled with CFR 0.152.
 */
import net.minecraft.server.MinecraftServer;

public class ov
implements ac {
    public static final ov a = new ov();
    private StringBuffer b = new StringBuffer();

    public void e() {
        this.b.setLength(0);
    }

    public String f() {
        return this.b.toString();
    }

    @Override
    public String b_() {
        return "Rcon";
    }

    @Override
    public fj c_() {
        return new fq(this.b_());
    }

    @Override
    public void a(fj fj2) {
        this.b.append(fj2.c());
    }

    @Override
    public boolean a(int n2, String string) {
        return true;
    }

    @Override
    public r f_() {
        return new r(0, 0, 0);
    }

    @Override
    public ahb d() {
        return MinecraftServer.I().d();
    }
}

