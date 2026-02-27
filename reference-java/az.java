/*
 * Decompiled with CFR 0.152.
 */
import java.util.Arrays;
import java.util.List;
import net.minecraft.server.MinecraftServer;

public class az
extends y {
    @Override
    public List b() {
        return Arrays.asList("w", "msg");
    }

    @Override
    public String c() {
        return "tell";
    }

    @Override
    public int a() {
        return 0;
    }

    @Override
    public String c(ac ac2) {
        return "commands.message.usage";
    }

    @Override
    public void b(ac ac2, String[] stringArray) {
        if (stringArray.length < 2) {
            throw new ci("commands.message.usage", new Object[0]);
        }
        mw mw2 = az.d(ac2, stringArray[0]);
        if (mw2 == null) {
            throw new cg();
        }
        if (mw2 == ac2) {
            throw new cg("commands.message.sameTarget", new Object[0]);
        }
        fj fj2 = az.a(ac2, stringArray, 1, !(ac2 instanceof yz));
        fr fr2 = new fr("commands.message.display.incoming", ac2.c_(), fj2.f());
        fr fr3 = new fr("commands.message.display.outgoing", mw2.c_(), fj2.f());
        fr2.b().a(a.h).b(true);
        fr3.b().a(a.h).b(true);
        mw2.a(fr2);
        ac2.a(fr3);
    }

    @Override
    public List a(ac ac2, String[] stringArray) {
        return az.a(stringArray, MinecraftServer.I().E());
    }

    @Override
    public boolean a(String[] stringArray, int n2) {
        return n2 == 0;
    }
}

