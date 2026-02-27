/*
 * Decompiled with CFR 0.152.
 */
import com.google.common.base.Charsets;
import com.google.common.collect.Lists;
import com.google.common.collect.Maps;
import com.google.common.io.Files;
import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.google.gson.JsonObject;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.IOException;
import java.io.Reader;
import java.lang.reflect.ParameterizedType;
import java.lang.reflect.Type;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Map;
import org.apache.commons.io.IOUtils;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class om {
    protected static final Logger a = LogManager.getLogger();
    protected final Gson b;
    private final File c;
    private final Map d = Maps.newHashMap();
    private boolean e = true;
    private static final ParameterizedType f = new on();

    public om(File file) {
        this.c = file;
        GsonBuilder gsonBuilder = new GsonBuilder().setPrettyPrinting();
        gsonBuilder.registerTypeHierarchyAdapter(ol.class, new oo(this, null));
        this.b = gsonBuilder.create();
    }

    public boolean b() {
        return this.e;
    }

    public void a(boolean bl2) {
        this.e = bl2;
    }

    public File c() {
        return this.c;
    }

    public void a(ol ol2) {
        this.d.put(this.a(ol2.f()), ol2);
        try {
            this.f();
        }
        catch (IOException iOException) {
            a.warn("Could not save the list after adding a user.", (Throwable)iOException);
        }
    }

    public ol b(Object object) {
        this.h();
        return (ol)this.d.get(this.a(object));
    }

    public void c(Object object) {
        this.d.remove(this.a(object));
        try {
            this.f();
        }
        catch (IOException iOException) {
            a.warn("Could not save the list after removing a user.", (Throwable)iOException);
        }
    }

    public String[] a() {
        return this.d.keySet().toArray(new String[this.d.size()]);
    }

    public boolean d() {
        return this.d.size() < 1;
    }

    protected String a(Object object) {
        return object.toString();
    }

    protected boolean d(Object object) {
        return this.d.containsKey(this.a(object));
    }

    private void h() {
        ArrayList<Object> arrayList = Lists.newArrayList();
        for (Object object : this.d.values()) {
            if (!((ol)object).e()) continue;
            arrayList.add(((ol)object).f());
        }
        for (Object object : arrayList) {
            this.d.remove(object);
        }
    }

    protected ol a(JsonObject jsonObject) {
        return new ol(null, jsonObject);
    }

    protected Map e() {
        return this.d;
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    public void f() {
        Collection collection = this.d.values();
        String string = this.b.toJson(collection);
        BufferedWriter bufferedWriter = null;
        try {
            bufferedWriter = Files.newWriter(this.c, Charsets.UTF_8);
            bufferedWriter.write(string);
        }
        catch (Throwable throwable) {
            IOUtils.closeQuietly(bufferedWriter);
            throw throwable;
        }
        IOUtils.closeQuietly(bufferedWriter);
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    public void g() {
        Collection collection = null;
        BufferedReader bufferedReader = null;
        try {
            bufferedReader = Files.newReader(this.c, Charsets.UTF_8);
            collection = (Collection)this.b.fromJson((Reader)bufferedReader, (Type)f);
        }
        catch (Throwable throwable) {
            IOUtils.closeQuietly(bufferedReader);
            throw throwable;
        }
        IOUtils.closeQuietly(bufferedReader);
        if (collection != null) {
            this.d.clear();
            for (ol ol2 : collection) {
                if (ol2.f() == null) continue;
                this.d.put(this.a(ol2.f()), ol2);
            }
        }
    }
}

