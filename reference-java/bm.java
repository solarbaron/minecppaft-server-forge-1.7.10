/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;

public class bm
extends y {
    @Override
    public String c() {
        return "setblock";
    }

    @Override
    public int a() {
        return 2;
    }

    @Override
    public String c(ac ac2) {
        return "commands.setblock.usage";
    }

    @Override
    public void b(ac ac2, String[] stringArray) {
        if (stringArray.length >= 4) {
            Object object;
            ahb ahb2;
            int n2 = ac2.f_().a;
            int n3 = ac2.f_().b;
            int n4 = ac2.f_().c;
            n2 = qh.c(bm.a(ac2, n2, stringArray[0]));
            n3 = qh.c(bm.a(ac2, n3, stringArray[1]));
            n4 = qh.c(bm.a(ac2, n4, stringArray[2]));
            aji aji2 = y.g(ac2, stringArray[3]);
            int n5 = 0;
            if (stringArray.length >= 5) {
                n5 = bm.a(ac2, stringArray[4], 0, 15);
            }
            if (!(ahb2 = ac2.d()).d(n2, n3, n4)) {
                throw new cd("commands.setblock.outOfWorld", new Object[0]);
            }
            dh dh2 = new dh();
            boolean bl2 = false;
            if (stringArray.length >= 7 && aji2.u()) {
                object = bm.a(ac2, stringArray, 6).c();
                try {
                    dy dy2 = eb.a((String)object);
                    if (!(dy2 instanceof dh)) {
                        throw new cd("commands.setblock.tagError", "Not a valid tag");
                    }
                    dh2 = (dh)dy2;
                    bl2 = true;
                }
                catch (ea ea2) {
                    throw new cd("commands.setblock.tagError", ea2.getMessage());
                }
            }
            if (stringArray.length >= 6) {
                if (stringArray[5].equals("destroy")) {
                    ahb2.a(n2, n3, n4, true);
                } else if (stringArray[5].equals("keep") && !ahb2.c(n2, n3, n4)) {
                    throw new cd("commands.setblock.noChange", new Object[0]);
                }
            }
            if (!ahb2.d(n2, n3, n4, aji2, n5, 3)) {
                throw new cd("commands.setblock.noChange", new Object[0]);
            }
            if (bl2 && (object = ahb2.o(n2, n3, n4)) != null) {
                dh2.a("x", n2);
                dh2.a("y", n3);
                dh2.a("z", n4);
                ((aor)object).a(dh2);
            }
            bm.a(ac2, (aa)this, "commands.setblock.success", new Object[0]);
            return;
        }
        throw new ci("commands.setblock.usage", new Object[0]);
    }

    @Override
    public List a(ac ac2, String[] stringArray) {
        if (stringArray.length == 4) {
            return bm.a(stringArray, aji.c.b());
        }
        if (stringArray.length == 6) {
            return bm.a(stringArray, "replace", "destroy", "keep");
        }
        return null;
    }
}

