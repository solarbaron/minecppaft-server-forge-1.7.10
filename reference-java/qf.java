/*
 * Decompiled with CFR 0.152.
 */
import com.google.gson.Gson;
import com.google.gson.TypeAdapter;
import com.google.gson.TypeAdapterFactory;
import com.google.gson.reflect.TypeToken;
import java.util.HashMap;
import java.util.Locale;

public class qf
implements TypeAdapterFactory {
    public TypeAdapter create(Gson gson, TypeToken typeToken) {
        Class clazz = typeToken.getRawType();
        if (!clazz.isEnum()) {
            return null;
        }
        HashMap hashMap = new HashMap();
        for (Object t2 : clazz.getEnumConstants()) {
            hashMap.put(this.a(t2), t2);
        }
        return new qg(this, hashMap);
    }

    private String a(Object object) {
        if (object instanceof Enum) {
            return ((Enum)object).name().toLowerCase(Locale.US);
        }
        return object.toString().toLowerCase(Locale.US);
    }

    static /* synthetic */ String a(qf qf2, Object object) {
        return qf2.a(object);
    }
}

