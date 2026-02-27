/*
 * Decompiled with CFR 0.152.
 */
public class aw
extends y {
    @Override
    public String c() {
        return "kill";
    }

    @Override
    public int a() {
        return 0;
    }

    @Override
    public String c(ac ac2) {
        return "commands.kill.usage";
    }

    @Override
    public void b(ac ac2, String[] stringArray) {
        mw mw2 = aw.b(ac2);
        ((yz)mw2).a(ro.i, Float.MAX_VALUE);
        ac2.a(new fr("commands.kill.success", new Object[0]));
    }
}

