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

class oo
implements JsonDeserializer,
JsonSerializer {
    final /* synthetic */ om a;

    private oo(om om2) {
        this.a = om2;
    }

    public JsonElement a(ol ol2, Type type, JsonSerializationContext jsonSerializationContext) {
        JsonObject jsonObject = new JsonObject();
        ol2.a(jsonObject);
        return jsonObject;
    }

    public ol a(JsonElement jsonElement, Type type, JsonDeserializationContext jsonDeserializationContext) {
        if (jsonElement.isJsonObject()) {
            JsonObject jsonObject = jsonElement.getAsJsonObject();
            ol ol2 = this.a.a(jsonObject);
            return ol2;
        }
        return null;
    }

    public /* synthetic */ JsonElement serialize(Object object, Type type, JsonSerializationContext jsonSerializationContext) {
        return this.a((ol)object, type, jsonSerializationContext);
    }

    public /* synthetic */ Object deserialize(JsonElement jsonElement, Type type, JsonDeserializationContext jsonDeserializationContext) {
        return this.a(jsonElement, type, jsonDeserializationContext);
    }

    /* synthetic */ oo(om om2, on on2) {
        this(om2);
    }
}

