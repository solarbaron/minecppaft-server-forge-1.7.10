/*
 * Decompiled with CFR 0.152.
 */
import com.google.gson.JsonObject;
import com.mojang.authlib.GameProfile;
import java.util.UUID;

public class ok
extends ol {
    private final int a;

    public ok(GameProfile gameProfile, int n2) {
        super(gameProfile);
        this.a = n2;
    }

    public ok(JsonObject jsonObject) {
        super(ok.b(jsonObject), jsonObject);
        this.a = jsonObject.has("level") ? jsonObject.get("level").getAsInt() : 0;
    }

    public int a() {
        return this.a;
    }

    @Override
    protected void a(JsonObject jsonObject) {
        if (this.f() == null) {
            return;
        }
        jsonObject.addProperty("uuid", ((GameProfile)this.f()).getId() == null ? "" : ((GameProfile)this.f()).getId().toString());
        jsonObject.addProperty("name", ((GameProfile)this.f()).getName());
        super.a(jsonObject);
        jsonObject.addProperty("level", this.a);
    }

    private static GameProfile b(JsonObject jsonObject) {
        UUID uUID;
        if (!jsonObject.has("uuid") || !jsonObject.has("name")) {
            return null;
        }
        String string = jsonObject.get("uuid").getAsString();
        try {
            uUID = UUID.fromString(string);
        }
        catch (Throwable throwable) {
            return null;
        }
        return new GameProfile(uUID, jsonObject.get("name").getAsString());
    }
}

