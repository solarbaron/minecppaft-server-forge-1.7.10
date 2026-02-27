/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;

public class bu
extends y {
    @Override
    public String c() {
        return "summon";
    }

    @Override
    public int a() {
        return 2;
    }

    @Override
    public String c(ac ac2) {
        return "commands.summon.usage";
    }

    @Override
    public void b(ac ac2, String[] stringArray) {
        if (stringArray.length >= 1) {
            Object object;
            Object object2;
            ahb ahb2;
            String string = stringArray[0];
            double d2 = (double)ac2.f_().a + 0.5;
            double d3 = ac2.f_().b;
            double d4 = (double)ac2.f_().c + 0.5;
            if (stringArray.length >= 4) {
                d2 = bu.a(ac2, d2, stringArray[1]);
                d3 = bu.a(ac2, d3, stringArray[2]);
                d4 = bu.a(ac2, d4, stringArray[3]);
            }
            if (!(ahb2 = ac2.d()).d((int)d2, (int)d3, (int)d4)) {
                bu.a(ac2, (aa)this, "commands.summon.outOfWorld", new Object[0]);
                return;
            }
            dh dh2 = new dh();
            boolean bl2 = false;
            if (stringArray.length >= 5) {
                object2 = bu.a(ac2, stringArray, 4);
                try {
                    object = eb.a(object2.c());
                    if (!(object instanceof dh)) {
                        bu.a(ac2, (aa)this, "commands.summon.tagError", "Not a valid tag");
                        return;
                    }
                    dh2 = (dh)object;
                    bl2 = true;
                }
                catch (ea ea2) {
                    bu.a(ac2, (aa)this, "commands.summon.tagError", ea2.getMessage());
                    return;
                }
            }
            dh2.a("id", string);
            object2 = sg.a(dh2, ahb2);
            if (object2 != null) {
                ((sa)object2).b(d2, d3, d4, ((sa)object2).y, ((sa)object2).z);
                if (!bl2 && object2 instanceof sw) {
                    ((sw)object2).a((sy)null);
                }
                ahb2.d((sa)object2);
                object = object2;
                dh dh3 = dh2;
                while (object != null && dh3.b("Riding", 10)) {
                    sa sa2 = sg.a(dh3.m("Riding"), ahb2);
                    if (sa2 != null) {
                        sa2.b(d2, d3, d4, sa2.y, sa2.z);
                        ahb2.d(sa2);
                        ((sa)object).a(sa2);
                    }
                    object = sa2;
                    dh3 = dh3.m("Riding");
                }
                bu.a(ac2, (aa)this, "commands.summon.success", new Object[0]);
                return;
            }
            bu.a(ac2, (aa)this, "commands.summon.failed", new Object[0]);
            return;
        }
        throw new ci("commands.summon.usage", new Object[0]);
    }

    @Override
    public List a(ac ac2, String[] stringArray) {
        if (stringArray.length == 1) {
            return bu.a(stringArray, this.d());
        }
        return null;
    }

    protected String[] d() {
        return sg.b().toArray(new String[0]);
    }
}

