/*
 * Decompiled with CFR 0.152.
 */
import com.mojang.authlib.GameProfile;

public class jw
extends ft {
    private GameProfile a;

    public jw() {
    }

    public jw(GameProfile gameProfile) {
        this.a = gameProfile;
    }

    @Override
    public void a(et et2) {
        this.a = new GameProfile(null, et2.c(16));
    }

    @Override
    public void b(et et2) {
        et2.a(this.a.getName());
    }

    public void a(jv jv2) {
        jv2.a(this);
    }

    public GameProfile c() {
        return this.a;
    }
}

