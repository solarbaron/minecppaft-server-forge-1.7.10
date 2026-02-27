/*
 * Decompiled with CFR 0.152.
 */
import com.google.gson.TypeAdapter;
import com.google.gson.stream.JsonReader;
import com.google.gson.stream.JsonToken;
import com.google.gson.stream.JsonWriter;
import java.util.Map;

class qg
extends TypeAdapter {
    final /* synthetic */ Map a;
    final /* synthetic */ qf b;

    qg(qf qf2, Map map) {
        this.b = qf2;
        this.a = map;
    }

    public void write(JsonWriter jsonWriter, Object object) {
        if (object == null) {
            jsonWriter.nullValue();
        } else {
            jsonWriter.value(qf.a(this.b, object));
        }
    }

    public Object read(JsonReader jsonReader) {
        if (jsonReader.peek() == JsonToken.NULL) {
            jsonReader.nextNull();
            return null;
        }
        return this.a.get(jsonReader.nextString());
    }
}

