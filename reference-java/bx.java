/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;

public class bx
extends y {
    @Override
    public String c() {
        return "testforblock";
    }

    @Override
    public int a() {
        return 2;
    }

    @Override
    public String c(ac ac2) {
        return "commands.testforblock.usage";
    }

    @Override
    public void b(ac ac2, String[] stringArray) {
        if (stringArray.length >= 4) {
            int n2;
            Object object;
            Object object2;
            ahb ahb2;
            int n3 = ac2.f_().a;
            int n4 = ac2.f_().b;
            int n5 = ac2.f_().c;
            n3 = qh.c(bx.a(ac2, n3, stringArray[0]));
            n4 = qh.c(bx.a(ac2, n4, stringArray[1]));
            n5 = qh.c(bx.a(ac2, n5, stringArray[2]));
            aji aji2 = aji.b(stringArray[3]);
            if (aji2 == null) {
                throw new ce("commands.setblock.notFound", stringArray[3]);
            }
            int n6 = -1;
            if (stringArray.length >= 5) {
                n6 = bx.a(ac2, stringArray[4], -1, 15);
            }
            if (!(ahb2 = ac2.d()).d(n3, n4, n5)) {
                throw new cd("commands.testforblock.outOfWorld", new Object[0]);
            }
            dh dh2 = new dh();
            boolean bl2 = false;
            if (stringArray.length >= 6 && aji2.u()) {
                object2 = bx.a(ac2, stringArray, 5).c();
                try {
                    object = eb.a((String)object2);
                    if (!(object instanceof dh)) {
                        throw new cd("commands.setblock.tagError", "Not a valid tag");
                    }
                    dh2 = (dh)object;
                    bl2 = true;
                }
                catch (ea ea2) {
                    throw new cd("commands.setblock.tagError", ea2.getMessage());
                }
            }
            if ((object2 = ahb2.a(n3, n4, n5)) != aji2) {
                throw new cd("commands.testforblock.failed.tile", n3, n4, n5, ((aji)object2).F(), aji2.F());
            }
            if (n6 > -1 && (n2 = ahb2.e(n3, n4, n5)) != n6) {
                throw new cd("commands.testforblock.failed.data", n3, n4, n5, n2, n6);
            }
            if (bl2) {
                object = ahb2.o(n3, n4, n5);
                if (object == null) {
                    throw new cd("commands.testforblock.failed.tileEntity", n3, n4, n5);
                }
                dh dh3 = new dh();
                ((aor)object).b(dh3);
                if (!this.a(dh2, dh3)) {
                    throw new cd("commands.testforblock.failed.nbt", n3, n4, n5);
                }
            }
            ac2.a(new fr("commands.testforblock.success", n3, n4, n5));
            return;
        }
        throw new ci("commands.testforblock.usage", new Object[0]);
    }

    public boolean a(dy dy2, dy dy3) {
        if (dy2 == dy3) {
            return true;
        }
        if (dy2 == null) {
            return true;
        }
        if (dy3 == null) {
            return false;
        }
        if (!dy2.getClass().equals(dy3.getClass())) {
            return false;
        }
        if (dy2 instanceof dh) {
            dh dh2 = (dh)dy2;
            dh dh3 = (dh)dy3;
            for (String string : dh2.c()) {
                dy dy4 = dh2.a(string);
                if (this.a(dy4, dh3.a(string))) continue;
                return false;
            }
            return true;
        }
        return dy2.equals(dy3);
    }

    @Override
    public List a(ac ac2, String[] stringArray) {
        if (stringArray.length == 4) {
            return bx.a(stringArray, aji.c.b());
        }
        return null;
    }
}

