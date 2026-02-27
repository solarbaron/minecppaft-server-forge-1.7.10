/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import net.minecraft.server.MinecraftServer;

public class as
extends y {
    @Override
    public String c() {
        return "gamerule";
    }

    @Override
    public int a() {
        return 2;
    }

    @Override
    public String c(ac ac2) {
        return "commands.gamerule.usage";
    }

    @Override
    public void b(ac ac2, String[] stringArray) {
        if (stringArray.length == 2) {
            String string = stringArray[0];
            String string2 = stringArray[1];
            agy agy2 = this.d();
            if (agy2.e(string)) {
                agy2.b(string, string2);
                as.a(ac2, (aa)this, "commands.gamerule.success", new Object[0]);
            } else {
                as.a(ac2, (aa)this, "commands.gamerule.norule", string);
            }
            return;
        }
        if (stringArray.length == 1) {
            String string = stringArray[0];
            agy agy3 = this.d();
            if (agy3.e(string)) {
                String string3 = agy3.a(string);
                ac2.a(new fq(string).a(" = ").a(string3));
            } else {
                as.a(ac2, (aa)this, "commands.gamerule.norule", string);
            }
            return;
        }
        if (stringArray.length == 0) {
            agy agy4 = this.d();
            ac2.a(new fq(as.a(agy4.b())));
            return;
        }
        throw new ci("commands.gamerule.usage", new Object[0]);
    }

    @Override
    public List a(ac ac2, String[] stringArray) {
        if (stringArray.length == 1) {
            return as.a(stringArray, this.d().b());
        }
        if (stringArray.length == 2) {
            return as.a(stringArray, "true", "false");
        }
        return null;
    }

    private agy d() {
        return MinecraftServer.I().a(0).O();
    }
}

