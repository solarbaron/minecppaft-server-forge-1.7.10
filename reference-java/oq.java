/*
 * Decompiled with CFR 0.152.
 */
import com.google.gson.JsonObject;
import com.mojang.authlib.GameProfile;
import java.util.Date;
import java.util.UUID;

public class oq
extends nr {
    public oq(GameProfile gameProfile) {
        this(gameProfile, null, null, null, null);
    }

    public oq(GameProfile gameProfile, Date date, String string, Date date2, String string2) {
        super(gameProfile, date2, string, date2, string2);
    }

    public oq(JsonObject jsonObject) {
        super(oq.b(jsonObject), jsonObject);
    }

    @Override
    protected void a(JsonObject jsonObject) {
        if (this.f() == null) {
            return;
        }
        jsonObject.addProperty("uuid", ((GameProfile)this.f()).getId() == null ? "" : ((GameProfile)this.f()).getId().toString());
        jsonObject.addProperty("name", ((GameProfile)this.f()).getName());
        super.a(jsonObject);
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

