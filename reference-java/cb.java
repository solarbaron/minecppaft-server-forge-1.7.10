/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;
import net.minecraft.server.MinecraftServer;

public class cb
extends y {
    @Override
    public String c() {
        return "weather";
    }

    @Override
    public int a() {
        return 2;
    }

    @Override
    public String c(ac ac2) {
        return "commands.weather.usage";
    }

    @Override
    public void b(ac ac2, String[] stringArray) {
        if (stringArray.length < 1 || stringArray.length > 2) {
            throw new ci("commands.weather.usage", new Object[0]);
        }
        int n2 = (300 + new Random().nextInt(600)) * 20;
        if (stringArray.length >= 2) {
            n2 = cb.a(ac2, stringArray[1], 1, 1000000) * 20;
        }
        mt mt2 = MinecraftServer.I().c[0];
        ays ays2 = mt2.N();
        if ("clear".equalsIgnoreCase(stringArray[0])) {
            ays2.g(0);
            ays2.f(0);
            ays2.b(false);
            ays2.a(false);
            cb.a(ac2, (aa)this, "commands.weather.clear", new Object[0]);
        } else if ("rain".equalsIgnoreCase(stringArray[0])) {
            ays2.g(n2);
            ays2.b(true);
            ays2.a(false);
            cb.a(ac2, (aa)this, "commands.weather.rain", new Object[0]);
        } else if ("thunder".equalsIgnoreCase(stringArray[0])) {
            ays2.g(n2);
            ays2.f(n2);
            ays2.b(true);
            ays2.a(true);
            cb.a(ac2, (aa)this, "commands.weather.thunder", new Object[0]);
        } else {
            throw new ci("commands.weather.usage", new Object[0]);
        }
    }

    @Override
    public List a(ac ac2, String[] stringArray) {
        if (stringArray.length == 1) {
            return cb.a(stringArray, "clear", "rain", "thunder");
        }
        return null;
    }
}

