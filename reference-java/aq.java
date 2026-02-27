/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import net.minecraft.server.MinecraftServer;

public class aq
extends y {
    @Override
    public String c() {
        return "difficulty";
    }

    @Override
    public int a() {
        return 2;
    }

    @Override
    public String c(ac ac2) {
        return "commands.difficulty.usage";
    }

    @Override
    public void b(ac ac2, String[] stringArray) {
        if (stringArray.length > 0) {
            rd rd2 = this.h(ac2, stringArray[0]);
            MinecraftServer.I().a(rd2);
            aq.a(ac2, (aa)this, "commands.difficulty.success", new fr(rd2.b(), new Object[0]));
            return;
        }
        throw new ci("commands.difficulty.usage", new Object[0]);
    }

    protected rd h(ac ac2, String string) {
        if (string.equalsIgnoreCase("peaceful") || string.equalsIgnoreCase("p")) {
            return rd.a;
        }
        if (string.equalsIgnoreCase("easy") || string.equalsIgnoreCase("e")) {
            return rd.b;
        }
        if (string.equalsIgnoreCase("normal") || string.equalsIgnoreCase("n")) {
            return rd.c;
        }
        if (string.equalsIgnoreCase("hard") || string.equalsIgnoreCase("h")) {
            return rd.d;
        }
        return rd.a(aq.a(ac2, string, 0, 3));
    }

    @Override
    public List a(ac ac2, String[] stringArray) {
        if (stringArray.length == 1) {
            return aq.a(stringArray, "peaceful", "easy", "normal", "hard");
        }
        return null;
    }
}

