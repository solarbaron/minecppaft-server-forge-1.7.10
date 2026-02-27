/*
 * Decompiled with CFR 0.152.
 */
import com.google.gson.JsonDeserializationContext;
import com.google.gson.JsonDeserializer;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonPrimitive;
import com.google.gson.JsonSerializationContext;
import com.google.gson.JsonSerializer;
import java.lang.reflect.Type;

public class fp
implements JsonDeserializer,
JsonSerializer {
    public fn a(JsonElement jsonElement, Type type, JsonDeserializationContext jsonDeserializationContext) {
        if (jsonElement.isJsonObject()) {
            Object object;
            fi fi2;
            JsonPrimitive jsonPrimitive;
            JsonObject jsonObject;
            fn fn2 = new fn();
            JsonObject jsonObject2 = jsonElement.getAsJsonObject();
            if (jsonObject2 == null) {
                return null;
            }
            if (jsonObject2.has("bold")) {
                fn.a(fn2, jsonObject2.get("bold").getAsBoolean());
            }
            if (jsonObject2.has("italic")) {
                fn.b(fn2, jsonObject2.get("italic").getAsBoolean());
            }
            if (jsonObject2.has("underlined")) {
                fn.c(fn2, jsonObject2.get("underlined").getAsBoolean());
            }
            if (jsonObject2.has("strikethrough")) {
                fn.d(fn2, jsonObject2.get("strikethrough").getAsBoolean());
            }
            if (jsonObject2.has("obfuscated")) {
                fn.e(fn2, jsonObject2.get("obfuscated").getAsBoolean());
            }
            if (jsonObject2.has("color")) {
                fn.a(fn2, (a)((Object)jsonDeserializationContext.deserialize(jsonObject2.get("color"), (Type)((Object)a.class))));
            }
            if (jsonObject2.has("clickEvent") && (jsonObject = jsonObject2.getAsJsonObject("clickEvent")) != null) {
                String string;
                jsonPrimitive = jsonObject.getAsJsonPrimitive("action");
                fi2 = jsonPrimitive == null ? null : fi.a(jsonPrimitive.getAsString());
                object = jsonObject.getAsJsonPrimitive("value");
                String string2 = string = object == null ? null : ((JsonPrimitive)object).getAsString();
                if (fi2 != null && string != null && fi2.a()) {
                    fn.a(fn2, new fh(fi2, string));
                }
            }
            if (jsonObject2.has("hoverEvent") && (jsonObject = jsonObject2.getAsJsonObject("hoverEvent")) != null) {
                jsonPrimitive = jsonObject.getAsJsonPrimitive("action");
                fi2 = jsonPrimitive == null ? null : fm.a(jsonPrimitive.getAsString());
                object = (fj)jsonDeserializationContext.deserialize(jsonObject.get("value"), (Type)((Object)fj.class));
                if (fi2 != null && object != null && ((fm)((Object)fi2)).a()) {
                    fn.a(fn2, new fl((fm)((Object)fi2), (fj)object));
                }
            }
            return fn2;
        }
        return null;
    }

    public JsonElement a(fn fn2, Type type, JsonSerializationContext jsonSerializationContext) {
        JsonObject jsonObject;
        if (fn2.g()) {
            return null;
        }
        JsonObject jsonObject2 = new JsonObject();
        if (fn.b(fn2) != null) {
            jsonObject2.addProperty("bold", fn.b(fn2));
        }
        if (fn.c(fn2) != null) {
            jsonObject2.addProperty("italic", fn.c(fn2));
        }
        if (fn.d(fn2) != null) {
            jsonObject2.addProperty("underlined", fn.d(fn2));
        }
        if (fn.e(fn2) != null) {
            jsonObject2.addProperty("strikethrough", fn.e(fn2));
        }
        if (fn.f(fn2) != null) {
            jsonObject2.addProperty("obfuscated", fn.f(fn2));
        }
        if (fn.g(fn2) != null) {
            jsonObject2.add("color", jsonSerializationContext.serialize((Object)fn.g(fn2)));
        }
        if (fn.h(fn2) != null) {
            jsonObject = new JsonObject();
            jsonObject.addProperty("action", fn.h(fn2).a().b());
            jsonObject.addProperty("value", fn.h(fn2).b());
            jsonObject2.add("clickEvent", jsonObject);
        }
        if (fn.i(fn2) != null) {
            jsonObject = new JsonObject();
            jsonObject.addProperty("action", fn.i(fn2).a().b());
            jsonObject.add("value", jsonSerializationContext.serialize(fn.i(fn2).b()));
            jsonObject2.add("hoverEvent", jsonObject);
        }
        return jsonObject2;
    }

    public /* synthetic */ JsonElement serialize(Object object, Type type, JsonSerializationContext jsonSerializationContext) {
        return this.a((fn)object, type, jsonSerializationContext);
    }

    public /* synthetic */ Object deserialize(JsonElement jsonElement, Type type, JsonDeserializationContext jsonDeserializationContext) {
        return this.a(jsonElement, type, jsonDeserializationContext);
    }
}

