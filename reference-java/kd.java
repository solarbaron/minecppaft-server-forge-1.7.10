/*
 * Decompiled with CFR 0.152.
 */
import com.google.gson.JsonArray;
import com.google.gson.JsonDeserializationContext;
import com.google.gson.JsonDeserializer;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonSerializationContext;
import com.google.gson.JsonSerializer;
import com.mojang.authlib.GameProfile;
import java.lang.reflect.Type;
import java.util.UUID;

public class kd
implements JsonDeserializer,
JsonSerializer {
    public kc a(JsonElement jsonElement, Type type, JsonDeserializationContext jsonDeserializationContext) {
        JsonArray jsonArray;
        JsonObject jsonObject = pu.l(jsonElement, "players");
        kc kc2 = new kc(pu.m(jsonObject, "max"), pu.m(jsonObject, "online"));
        if (pu.d(jsonObject, "sample") && (jsonArray = pu.t(jsonObject, "sample")).size() > 0) {
            GameProfile[] gameProfileArray = new GameProfile[jsonArray.size()];
            for (int i2 = 0; i2 < gameProfileArray.length; ++i2) {
                JsonObject jsonObject2 = pu.l(jsonArray.get(i2), "player[" + i2 + "]");
                String string = pu.h(jsonObject2, "id");
                gameProfileArray[i2] = new GameProfile(UUID.fromString(string), pu.h(jsonObject2, "name"));
            }
            kc2.a(gameProfileArray);
        }
        return kc2;
    }

    public JsonElement a(kc kc2, Type type, JsonSerializationContext jsonSerializationContext) {
        JsonObject jsonObject = new JsonObject();
        jsonObject.addProperty("max", kc2.a());
        jsonObject.addProperty("online", kc2.b());
        if (kc2.c() != null && kc2.c().length > 0) {
            JsonArray jsonArray = new JsonArray();
            for (int i2 = 0; i2 < kc2.c().length; ++i2) {
                JsonObject jsonObject2 = new JsonObject();
                UUID uUID = kc2.c()[i2].getId();
                jsonObject2.addProperty("id", uUID == null ? "" : uUID.toString());
                jsonObject2.addProperty("name", kc2.c()[i2].getName());
                jsonArray.add(jsonObject2);
            }
            jsonObject.add("sample", jsonArray);
        }
        return jsonObject;
    }

    public /* synthetic */ JsonElement serialize(Object object, Type type, JsonSerializationContext jsonSerializationContext) {
        return this.a((kc)object, type, jsonSerializationContext);
    }

    public /* synthetic */ Object deserialize(JsonElement jsonElement, Type type, JsonDeserializationContext jsonDeserializationContext) {
        return this.a(jsonElement, type, jsonDeserializationContext);
    }
}

