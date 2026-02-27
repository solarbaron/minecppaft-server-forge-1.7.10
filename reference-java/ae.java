/*
 * Decompiled with CFR 0.152.
 */
import com.google.common.collect.Lists;
import java.util.ArrayList;
import java.util.List;
import net.minecraft.server.MinecraftServer;

public class ae
extends y {
    @Override
    public String c() {
        return "achievement";
    }

    @Override
    public int a() {
        return 2;
    }

    @Override
    public String c(ac ac2) {
        return "commands.achievement.usage";
    }

    @Override
    public void b(ac ac2, String[] stringArray) {
        if (stringArray.length >= 2) {
            ph ph2 = pp.a(stringArray[1]);
            if (ph2 == null && !stringArray[1].equals("*")) {
                throw new cd("commands.achievement.unknownAchievement", stringArray[1]);
            }
            mw mw2 = stringArray.length >= 3 ? ae.d(ac2, stringArray[2]) : ae.b(ac2);
            if (stringArray[0].equalsIgnoreCase("give")) {
                if (ph2 == null) {
                    for (pb pb2 : pc.e) {
                        mw2.a(pb2);
                    }
                    ae.a(ac2, (aa)this, "commands.achievement.give.success.all", mw2.b_());
                } else {
                    if (ph2 instanceof pb) {
                        pb pb3 = (pb)ph2;
                        ArrayList<pb> arrayList = Lists.newArrayList();
                        while (pb3.c != null && !mw2.w().a(pb3.c)) {
                            arrayList.add(pb3.c);
                            pb3 = pb3.c;
                        }
                        for (pb pb4 : Lists.reverse(arrayList)) {
                            mw2.a(pb4);
                        }
                    }
                    mw2.a(ph2);
                    ae.a(ac2, (aa)this, "commands.achievement.give.success.one", mw2.b_(), ph2.j());
                }
                return;
            }
        }
        throw new ci("commands.achievement.usage", new Object[0]);
    }

    @Override
    public List a(ac ac2, String[] stringArray) {
        if (stringArray.length == 1) {
            return ae.a(stringArray, "give");
        }
        if (stringArray.length == 2) {
            ArrayList<String> arrayList = Lists.newArrayList();
            for (ph ph2 : pp.b) {
                arrayList.add(ph2.e);
            }
            return ae.a(stringArray, arrayList);
        }
        if (stringArray.length == 3) {
            return ae.a(stringArray, MinecraftServer.I().E());
        }
        return null;
    }

    @Override
    public boolean a(String[] stringArray, int n2) {
        return n2 == 2;
    }
}

