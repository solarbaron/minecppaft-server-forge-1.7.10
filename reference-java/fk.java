/*
 * Decompiled with CFR 0.152.
 */
import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.google.gson.JsonArray;
import com.google.gson.JsonDeserializationContext;
import com.google.gson.JsonDeserializer;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonParseException;
import com.google.gson.JsonPrimitive;
import com.google.gson.JsonSerializationContext;
import com.google.gson.JsonSerializer;
import java.lang.reflect.Type;
import java.util.Map;

public class fk
implements JsonDeserializer,
JsonSerializer {
    private static final Gson a;

    public fj a(JsonElement jsonElement, Type type, JsonDeserializationContext jsonDeserializationContext) {
        if (jsonElement.isJsonPrimitive()) {
            return new fq(jsonElement.getAsString());
        }
        if (jsonElement.isJsonObject()) {
            Object object;
            fe fe2;
            JsonObject jsonObject = jsonElement.getAsJsonObject();
            if (jsonObject.has("text")) {
                fe2 = new fq(jsonObject.get("text").getAsString());
            } else if (jsonObject.has("translate")) {
                object = jsonObject.get("translate").getAsString();
                if (jsonObject.has("with")) {
                    JsonArray jsonArray = jsonObject.getAsJsonArray("with");
                    Object[] objectArray = new Object[jsonArray.size()];
                    for (int i2 = 0; i2 < objectArray.length; ++i2) {
                        fq fq2;
                        objectArray[i2] = this.a(jsonArray.get(i2), type, jsonDeserializationContext);
                        if (!(objectArray[i2] instanceof fq) || !(fq2 = (fq)objectArray[i2]).b().g() || !fq2.a().isEmpty()) continue;
                        objectArray[i2] = fq2.g();
                    }
                    fe2 = new fr((String)object, objectArray);
                } else {
                    fe2 = new fr((String)object, new Object[0]);
                }
            } else {
                throw new JsonParseException("Don't know how to turn " + jsonElement.toString() + " into a Component");
            }
            if (jsonObject.has("extra")) {
                object = jsonObject.getAsJsonArray("extra");
                if (((JsonArray)object).size() > 0) {
                    for (int i3 = 0; i3 < ((JsonArray)object).size(); ++i3) {
                        fe2.a(this.a(((JsonArray)object).get(i3), type, jsonDeserializationContext));
                    }
                } else {
                    throw new JsonParseException("Unexpected empty array of components");
                }
            }
            fe2.a((fn)jsonDeserializationContext.deserialize(jsonElement, (Type)((Object)fn.class)));
            return fe2;
        }
        if (jsonElement.isJsonArray()) {
            JsonArray jsonArray = jsonElement.getAsJsonArray();
            fj fj2 = null;
            for (JsonElement jsonElement2 : jsonArray) {
                fj fj3 = this.a(jsonElement2, jsonElement2.getClass(), jsonDeserializationContext);
                if (fj2 == null) {
                    fj2 = fj3;
                    continue;
                }
                fj2.a(fj3);
            }
            return fj2;
        }
        throw new JsonParseException("Don't know how to turn " + jsonElement.toString() + " into a Component");
    }

    private void a(fn fn2, JsonObject jsonObject, JsonSerializationContext jsonSerializationContext) {
        JsonElement jsonElement = jsonSerializationContext.serialize(fn2);
        if (jsonElement.isJsonObject()) {
            JsonObject jsonObject2 = (JsonObject)jsonElement;
            for (Map.Entry<String, JsonElement> entry : jsonObject2.entrySet()) {
                jsonObject.add(entry.getKey(), entry.getValue());
            }
        }
    }

    public JsonElement a(fj fj2, Type type, JsonSerializationContext jsonSerializationContext) {
        Iterable<JsonElement> iterable;
        if (fj2 instanceof fq && fj2.b().g() && fj2.a().isEmpty()) {
            return new JsonPrimitive(((fq)fj2).g());
        }
        JsonObject jsonObject = new JsonObject();
        if (!fj2.b().g()) {
            this.a(fj2.b(), jsonObject, jsonSerializationContext);
        }
        if (!fj2.a().isEmpty()) {
            iterable = new JsonArray();
            for (Object[] objectArray : fj2.a()) {
                ((JsonArray)iterable).add(this.a((fj)objectArray, objectArray.getClass(), jsonSerializationContext));
            }
            jsonObject.add("extra", (JsonElement)((Object)iterable));
        }
        if (fj2 instanceof fq) {
            jsonObject.addProperty("text", ((fq)fj2).g());
        } else if (fj2 instanceof fr) {
            iterable = (fr)fj2;
            jsonObject.addProperty("translate", ((fr)iterable).i());
            if (((fr)iterable).j() != null && ((fr)iterable).j().length > 0) {
                JsonArray jsonArray = new JsonArray();
                for (Object object : ((fr)iterable).j()) {
                    if (object instanceof fj) {
                        jsonArray.add(this.a((fj)object, object.getClass(), jsonSerializationContext));
                        continue;
                    }
                    jsonArray.add(new JsonPrimitive(String.valueOf(object)));
                }
                jsonObject.add("with", jsonArray);
            }
        } else {
            throw new IllegalArgumentException("Don't know how to serialize " + fj2 + " as a Component");
        }
        return jsonObject;
    }

    public static String a(fj fj2) {
        return a.toJson(fj2);
    }

    public static fj a(String string) {
        return a.fromJson(string, fj.class);
    }

    public /* synthetic */ JsonElement serialize(Object object, Type type, JsonSerializationContext jsonSerializationContext) {
        return this.a((fj)object, type, jsonSerializationContext);
    }

    public /* synthetic */ Object deserialize(JsonElement jsonElement, Type type, JsonDeserializationContext jsonDeserializationContext) {
        return this.a(jsonElement, type, jsonDeserializationContext);
    }

    static {
        GsonBuilder gsonBuilder = new GsonBuilder();
        gsonBuilder.registerTypeHierarchyAdapter(fj.class, new fk());
        gsonBuilder.registerTypeHierarchyAdapter(fn.class, new fp());
        gsonBuilder.registerTypeAdapterFactory(new qf());
        a = gsonBuilder.create();
    }
}

