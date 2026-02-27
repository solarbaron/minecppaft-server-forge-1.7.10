/*
 * Decompiled with CFR 0.152.
 */
import net.minecraft.server.MinecraftServer;

public class bl
extends z
implements x {
    public bl() {
        this.a(new bz());
        this.a(new ar());
        this.a(new aq());
        this.a(new al());
        this.a(new aw());
        this.a(new ca());
        this.a(new cb());
        this.a(new ap());
        this.a(new bv());
        this.a(new at());
        this.a(new am());
        this.a(new ao());
        this.a(new an());
        this.a(new bq());
        this.a(new au());
        this.a(new ak());
        this.a(new az());
        this.a(new ah());
        this.a(new bp());
        this.a(new bn());
        this.a(new as());
        this.a(new ai());
        this.a(new by());
        this.a(new br());
        this.a(new bf());
        this.a(new bk());
        this.a(new ae());
        this.a(new bu());
        this.a(new bm());
        this.a(new bx());
        this.a(new ba());
        if (MinecraftServer.I().X()) {
            this.a(new bc());
            this.a(new aj());
            this.a(new bt());
            this.a(new bh());
            this.a(new bi());
            this.a(new bj());
            this.a(new af());
            this.a(new bd());
            this.a(new ag());
            this.a(new ax());
            this.a(new be());
            this.a(new av());
            this.a(new ay());
            this.a(new cc());
            this.a(new bo());
            this.a(new bb());
        } else {
            this.a(new bg());
        }
        y.a(this);
    }

    @Override
    public void a(ac ac2, aa aa2, int n2, String string, Object ... objectArray) {
        boolean bl2 = true;
        if (ac2 instanceof agp && !MinecraftServer.I().c[0].O().b("commandBlockOutput")) {
            bl2 = false;
        }
        fr fr2 = new fr("chat.type.admin", ac2.b_(), new fr(string, objectArray));
        fr2.b().a(a.h);
        fr2.b().b(true);
        if (bl2) {
            for (yz yz2 : MinecraftServer.I().ah().e) {
                if (yz2 == ac2 || !MinecraftServer.I().ah().g(yz2.bJ()) || !aa2.a(yz2) || ac2 instanceof ov && !MinecraftServer.I().m()) continue;
                yz2.a(fr2);
            }
        }
        if (ac2 != MinecraftServer.I()) {
            MinecraftServer.I().a(fr2);
        }
        if ((n2 & 1) != 1) {
            ac2.a(new fr(string, objectArray));
        }
    }
}

