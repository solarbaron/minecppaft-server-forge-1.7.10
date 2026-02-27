/*
 * Decompiled with CFR 0.152.
 */
import com.google.common.collect.ForwardingSet;
import com.google.common.collect.Sets;
import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonPrimitive;
import java.util.Set;

public class pr
extends ForwardingSet
implements po {
    private final Set a = Sets.newHashSet();

    @Override
    public void a(JsonElement jsonElement) {
        if (jsonElement.isJsonArray()) {
            for (JsonElement jsonElement2 : jsonElement.getAsJsonArray()) {
                this.add(jsonElement2.getAsString());
            }
        }
    }

    @Override
    public JsonElement a() {
        JsonArray jsonArray = new JsonArray();
        for (String string : this) {
            jsonArray.add(new JsonPrimitive(string));
        }
        return jsonArray;
    }

    @Override
    protected Set delegate() {
        return this.a;
    }
}

