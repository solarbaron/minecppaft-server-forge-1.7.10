/*
 * Decompiled with CFR 0.152.
 */
import com.google.gson.JsonObject;
import java.util.Date;

public class ny
extends nr {
    public ny(String string) {
        this(string, null, null, null, null);
    }

    public ny(String string, Date date, String string2, Date date2, String string3) {
        super(string, date, string2, date2, string3);
    }

    public ny(JsonObject jsonObject) {
        super(ny.b(jsonObject), jsonObject);
    }

    private static String b(JsonObject jsonObject) {
        return jsonObject.has("ip") ? jsonObject.get("ip").getAsString() : null;
    }

    @Override
    protected void a(JsonObject jsonObject) {
        if (this.f() == null) {
            return;
        }
        jsonObject.addProperty("ip", (String)this.f());
        super.a(jsonObject);
    }
}

