/*
 * Decompiled with CFR 0.152.
 */
import java.lang.reflect.ParameterizedType;
import java.lang.reflect.Type;
import java.util.List;

final class on
implements ParameterizedType {
    on() {
    }

    @Override
    public Type[] getActualTypeArguments() {
        return new Type[]{ol.class};
    }

    @Override
    public Type getRawType() {
        return List.class;
    }

    @Override
    public Type getOwnerType() {
        return null;
    }
}

