/*
 * Decompiled with CFR 0.152.
 */
import com.google.gson.JsonDeserializationContext;
import com.google.gson.JsonDeserializer;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonSerializationContext;
import com.google.gson.JsonSerializer;
import java.lang.reflect.Type;

public class ke
implements JsonDeserializer,
JsonSerializer {
    public kb a(JsonElement jsonElement, Type type, JsonDeserializationContext jsonDeserializationContext) {
        JsonObject jsonObject = pu.l(jsonElement, "status");
        kb kb2 = new kb();
        if (jsonObject.has("description")) {
            kb2.a((fj)jsonDeserializationContext.deserialize(jsonObject.get("description"), (Type)((Object)fj.class)));
        }
        if (jsonObject.has("players")) {
            kb2.a((kc)jsonDeserializationContext.deserialize(jsonObject.get("players"), (Type)((Object)kc.class)));
        }
        if (jsonObject.has("version")) {
            kb2.a((kf)jsonDeserializationContext.deserialize(jsonObject.get("version"), (Type)((Object)kf.class)));
        }
        if (jsonObject.has("favicon")) {
            kb2.a(pu.h(jsonObject, "favicon"));
        }
        return kb2;
    }

    public JsonElement a(kb kb2, Type type, JsonSerializationContext jsonSerializationContext) {
        JsonObject jsonObject = new JsonObject();
        if (kb2.a() != null) {
            jsonObject.add("description", jsonSerializationContext.serialize(kb2.a()));
        }
        if (kb2.b() != null) {
            jsonObject.add("players", jsonSerializationContext.serialize(kb2.b()));
        }
        if (kb2.c() != null) {
            jsonObject.add("version", jsonSerializationContext.serialize(kb2.c()));
        }
        if (kb2.d() != null) {
            jsonObject.addProperty("favicon", kb2.d());
        }
        return jsonObject;
    }

    public /* synthetic */ JsonElement serialize(Object object, Type type, JsonSerializationContext jsonSerializationContext) {
        return this.a((kb)object, type, jsonSerializationContext);
    }

    public /* synthetic */ Object deserialize(JsonElement jsonElement, Type type, JsonDeserializationContext jsonDeserializationContext) {
        return this.a(jsonElement, type, jsonDeserializationContext);
    }
}

