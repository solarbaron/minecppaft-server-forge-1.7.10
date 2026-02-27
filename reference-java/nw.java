/*
 * Decompiled with CFR 0.152.
 */
import com.google.gson.JsonDeserializationContext;
import com.google.gson.JsonDeserializer;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonSerializationContext;
import com.google.gson.JsonSerializer;
import com.mojang.authlib.GameProfile;
import java.lang.reflect.Type;
import java.text.ParseException;
import java.util.Date;
import java.util.UUID;

class nw
implements JsonDeserializer,
JsonSerializer {
    final /* synthetic */ ns a;

    private nw(ns ns2) {
        this.a = ns2;
    }

    public JsonElement a(nv nv2, Type type, JsonSerializationContext jsonSerializationContext) {
        JsonObject jsonObject = new JsonObject();
        jsonObject.addProperty("name", nv2.a().getName());
        UUID uUID = nv2.a().getId();
        jsonObject.addProperty("uuid", uUID == null ? "" : uUID.toString());
        jsonObject.addProperty("expiresOn", ns.a.format(nv2.b()));
        return jsonObject;
    }

    public nv a(JsonElement jsonElement, Type type, JsonDeserializationContext jsonDeserializationContext) {
        if (jsonElement.isJsonObject()) {
            UUID uUID;
            JsonObject jsonObject = jsonElement.getAsJsonObject();
            JsonElement jsonElement2 = jsonObject.get("name");
            JsonElement jsonElement3 = jsonObject.get("uuid");
            JsonElement jsonElement4 = jsonObject.get("expiresOn");
            if (jsonElement2 == null || jsonElement3 == null) {
                return null;
            }
            String string = jsonElement3.getAsString();
            String string2 = jsonElement2.getAsString();
            Date date = null;
            if (jsonElement4 != null) {
                try {
                    date = ns.a.parse(jsonElement4.getAsString());
                }
                catch (ParseException parseException) {
                    date = null;
                }
            }
            if (string2 == null || string == null) {
                return null;
            }
            try {
                uUID = UUID.fromString(string);
            }
            catch (Throwable throwable) {
                return null;
            }
            nv nv2 = new nv(this.a, new GameProfile(uUID, string2), date, null);
            return nv2;
        }
        return null;
    }

    public /* synthetic */ JsonElement serialize(Object object, Type type, JsonSerializationContext jsonSerializationContext) {
        return this.a((nv)object, type, jsonSerializationContext);
    }

    public /* synthetic */ Object deserialize(JsonElement jsonElement, Type type, JsonDeserializationContext jsonDeserializationContext) {
        return this.a(jsonElement, type, jsonDeserializationContext);
    }

    /* synthetic */ nw(ns ns2, nt nt2) {
        this(ns2);
    }
}

