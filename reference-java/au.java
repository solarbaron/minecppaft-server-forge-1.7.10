/*
 * Decompiled with CFR 0.152.
 */
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import net.minecraft.server.MinecraftServer;

public class au
extends y {
    @Override
    public String c() {
        return "help";
    }

    @Override
    public int a() {
        return 0;
    }

    @Override
    public String c(ac ac2) {
        return "commands.help.usage";
    }

    @Override
    public List b() {
        return Arrays.asList("?");
    }

    @Override
    public void b(ac ac2, String[] stringArray) {
        List list = this.d(ac2);
        int n2 = 7;
        int n3 = (list.size() - 1) / n2;
        int n4 = 0;
        try {
            n4 = stringArray.length == 0 ? 0 : au.a(ac2, stringArray[0], 1, n3 + 1) - 1;
        }
        catch (ce ce2) {
            Map map = this.d();
            aa aa2 = (aa)map.get(stringArray[0]);
            if (aa2 != null) {
                throw new ci(aa2.c(ac2), new Object[0]);
            }
            if (qh.a(stringArray[0], -1) != -1) {
                throw ce2;
            }
            throw new ch();
        }
        int n5 = Math.min((n4 + 1) * n2, list.size());
        fr fr2 = new fr("commands.help.header", n4 + 1, n3 + 1);
        fr2.b().a(a.c);
        ac2.a(fr2);
        for (int i2 = n4 * n2; i2 < n5; ++i2) {
            aa aa3 = (aa)list.get(i2);
            fr fr3 = new fr(aa3.c(ac2), new Object[0]);
            fr3.b().a(new fh(fi.e, "/" + aa3.c() + " "));
            ac2.a(fr3);
        }
        if (n4 == 0 && ac2 instanceof yz) {
            fr fr4 = new fr("commands.help.footer", new Object[0]);
            fr4.b().a(a.k);
            ac2.a(fr4);
        }
    }

    protected List d(ac ac2) {
        List list = MinecraftServer.I().J().a(ac2);
        Collections.sort(list);
        return list;
    }

    protected Map d() {
        return MinecraftServer.I().J().a();
    }
}

