/*
 * Decompiled with CFR 0.152.
 */
import com.google.common.collect.Iterables;
import com.mojang.authlib.GameProfile;
import com.mojang.authlib.properties.Property;
import net.minecraft.server.MinecraftServer;

public class apn
extends aor {
    private int a;
    private int i;
    private GameProfile j = null;

    @Override
    public void b(dh dh2) {
        super.b(dh2);
        dh2.a("SkullType", (byte)(this.a & 0xFF));
        dh2.a("Rot", (byte)(this.i & 0xFF));
        if (this.j != null) {
            dh dh3 = new dh();
            dv.a(dh3, this.j);
            dh2.a("Owner", dh3);
        }
    }

    @Override
    public void a(dh dh2) {
        super.a(dh2);
        this.a = dh2.d("SkullType");
        this.i = dh2.d("Rot");
        if (this.a == 3) {
            if (dh2.b("Owner", 10)) {
                this.j = dv.a(dh2.m("Owner"));
            } else if (dh2.b("ExtraType", 8) && !qn.b(dh2.j("ExtraType"))) {
                this.j = new GameProfile(null, dh2.j("ExtraType"));
                this.d();
            }
        }
    }

    public GameProfile a() {
        return this.j;
    }

    @Override
    public ft m() {
        dh dh2 = new dh();
        this.b(dh2);
        return new gf(this.c, this.d, this.e, 4, dh2);
    }

    public void a(int n2) {
        this.a = n2;
        this.j = null;
    }

    public void a(GameProfile gameProfile) {
        this.a = 3;
        this.j = gameProfile;
        this.d();
    }

    private void d() {
        if (this.j == null || qn.b(this.j.getName())) {
            return;
        }
        if (this.j.isComplete() && this.j.getProperties().containsKey("textures")) {
            return;
        }
        GameProfile gameProfile = MinecraftServer.I().ax().a(this.j.getName());
        if (gameProfile == null) {
            return;
        }
        Property property = Iterables.getFirst(gameProfile.getProperties().get("textures"), null);
        if (property == null) {
            gameProfile = MinecraftServer.I().av().fillProfileProperties(gameProfile, true);
        }
        this.j = gameProfile;
        this.e();
    }

    public int b() {
        return this.a;
    }

    public void b(int n2) {
        this.i = n2;
    }
}

