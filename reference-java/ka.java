/*
 * Decompiled with CFR 0.152.
 */
import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import java.lang.reflect.Type;

public class ka
extends ft {
    private static final Gson a = new GsonBuilder().registerTypeAdapter((Type)((Object)kf.class), new kg()).registerTypeAdapter((Type)((Object)kc.class), new kd()).registerTypeAdapter((Type)((Object)kb.class), new ke()).registerTypeHierarchyAdapter(fj.class, new fk()).registerTypeHierarchyAdapter(fn.class, new fp()).registerTypeAdapterFactory(new qf()).create();
    private kb b;

    public ka() {
    }

    public ka(kb kb2) {
        this.b = kb2;
    }

    @Override
    public void a(et et2) {
        this.b = a.fromJson(et2.c(Short.MAX_VALUE), kb.class);
    }

    @Override
    public void b(et et2) {
        et2.a(a.toJson(this.b));
    }

    public void a(jy jy2) {
        jy2.a(this);
    }

    @Override
    public boolean a() {
        return true;
    }
}

