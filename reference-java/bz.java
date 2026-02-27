/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import net.minecraft.server.MinecraftServer;

public class bz
extends y {
    @Override
    public String c() {
        return "time";
    }

    @Override
    public int a() {
        return 2;
    }

    @Override
    public String c(ac ac2) {
        return "commands.time.usage";
    }

    @Override
    public void b(ac ac2, String[] stringArray) {
        if (stringArray.length > 1) {
            if (stringArray[0].equals("set")) {
                int n2 = stringArray[1].equals("day") ? 1000 : (stringArray[1].equals("night") ? 13000 : bz.a(ac2, stringArray[1], 0));
                this.a(ac2, n2);
                bz.a(ac2, (aa)this, "commands.time.set", n2);
                return;
            }
            if (stringArray[0].equals("add")) {
                int n3 = bz.a(ac2, stringArray[1], 0);
                this.b(ac2, n3);
                bz.a(ac2, (aa)this, "commands.time.added", n3);
                return;
            }
        }
        throw new ci("commands.time.usage", new Object[0]);
    }

    @Override
    public List a(ac ac2, String[] stringArray) {
        if (stringArray.length == 1) {
            return bz.a(stringArray, "set", "add");
        }
        if (stringArray.length == 2 && stringArray[0].equals("set")) {
            return bz.a(stringArray, "day", "night");
        }
        return null;
    }

    protected void a(ac ac2, int n2) {
        for (int i2 = 0; i2 < MinecraftServer.I().c.length; ++i2) {
            MinecraftServer.I().c[i2].b(n2);
        }
    }

    protected void b(ac ac2, int n2) {
        for (int i2 = 0; i2 < MinecraftServer.I().c.length; ++i2) {
            mt mt2 = MinecraftServer.I().c[i2];
            mt2.b(mt2.J() + (long)n2);
        }
    }
}

