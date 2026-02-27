/*
 * Decompiled with CFR 0.152.
 */
import com.mojang.authlib.GameProfile;
import java.util.UUID;

public class js
extends ft {
    private GameProfile a;

    public js() {
    }

    public js(GameProfile gameProfile) {
        this.a = gameProfile;
    }

    @Override
    public void a(et et2) {
        String string = et2.c(36);
        String string2 = et2.c(16);
        UUID uUID = UUID.fromString(string);
        this.a = new GameProfile(uUID, string2);
    }

    @Override
    public void b(et et2) {
        UUID uUID = this.a.getId();
        et2.a(uUID == null ? "" : uUID.toString());
        et2.a(this.a.getName());
    }

    public void a(jr jr2) {
        jr2.a(this);
    }

    @Override
    public boolean a() {
        return true;
    }
}

