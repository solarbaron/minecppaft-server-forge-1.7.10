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

public class kg
implements JsonDeserializer,
JsonSerializer {
    public kf a(JsonElement jsonElement, Type type, JsonDeserializationContext jsonDeserializationContext) {
        JsonObject jsonObject = pu.l(jsonElement, "version");
        return new kf(pu.h(jsonObject, "name"), pu.m(jsonObject, "protocol"));
    }

    public JsonElement a(kf kf2, Type type, JsonSerializationContext jsonSerializationContext) {
        JsonObject jsonObject = new JsonObject();
        jsonObject.addProperty("name", kf2.a());
        jsonObject.addProperty("protocol", kf2.b());
        return jsonObject;
    }

    public /* synthetic */ JsonElement serialize(Object object, Type type, JsonSerializationContext jsonSerializationContext) {
        return this.a((kf)object, type, jsonSerializationContext);
    }

    public /* synthetic */ Object deserialize(JsonElement jsonElement, Type type, JsonDeserializationContext jsonDeserializationContext) {
        return this.a(jsonElement, type, jsonDeserializationContext);
    }
}

