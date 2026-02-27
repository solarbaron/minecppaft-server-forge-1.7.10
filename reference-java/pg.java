/*
 * Decompiled with CFR 0.152.
 */
import com.google.common.collect.Maps;
import com.google.common.collect.Sets;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonParseException;
import com.google.gson.JsonParser;
import java.io.File;
import java.io.IOException;
import java.lang.reflect.Constructor;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import net.minecraft.server.MinecraftServer;
import org.apache.commons.io.FileUtils;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class pg
extends pq {
    private static final Logger b = LogManager.getLogger();
    private final MinecraftServer c;
    private final File d;
    private final Set e = Sets.newHashSet();
    private int f = -300;
    private boolean g = false;

    public pg(MinecraftServer minecraftServer, File file) {
        this.c = minecraftServer;
        this.d = file;
    }

    public void a() {
        if (this.d.isFile()) {
            try {
                this.a.clear();
                this.a.putAll(this.a(FileUtils.readFileToString(this.d)));
            }
            catch (IOException iOException) {
                b.error("Couldn't read statistics file " + this.d, (Throwable)iOException);
            }
            catch (JsonParseException jsonParseException) {
                b.error("Couldn't parse statistics file " + this.d, (Throwable)jsonParseException);
            }
        }
    }

    public void b() {
        try {
            FileUtils.writeStringToFile(this.d, pg.a(this.a));
        }
        catch (IOException iOException) {
            b.error("Couldn't save stats", (Throwable)iOException);
        }
    }

    @Override
    public void a(yz yz2, ph ph2, int n2) {
        int n3 = ph2.d() ? this.a(ph2) : 0;
        super.a(yz2, ph2, n2);
        this.e.add(ph2);
        if (ph2.d() && n3 == 0 && n2 > 0) {
            this.g = true;
            if (this.c.at()) {
                this.c.ah().a(new fr("chat.type.achievement", yz2.c_(), ph2.j()));
            }
        }
    }

    public Set c() {
        HashSet hashSet = Sets.newHashSet(this.e);
        this.e.clear();
        this.g = false;
        return hashSet;
    }

    public Map a(String string) {
        JsonElement jsonElement = new JsonParser().parse(string);
        if (!jsonElement.isJsonObject()) {
            return Maps.newHashMap();
        }
        JsonObject jsonObject = jsonElement.getAsJsonObject();
        HashMap<ph, pn> hashMap = Maps.newHashMap();
        for (Map.Entry<String, JsonElement> entry : jsonObject.entrySet()) {
            ph ph2 = pp.a(entry.getKey());
            if (ph2 != null) {
                pn pn2 = new pn();
                if (entry.getValue().isJsonPrimitive() && entry.getValue().getAsJsonPrimitive().isNumber()) {
                    pn2.a(entry.getValue().getAsInt());
                } else if (entry.getValue().isJsonObject()) {
                    JsonObject jsonObject2 = entry.getValue().getAsJsonObject();
                    if (jsonObject2.has("value") && jsonObject2.get("value").isJsonPrimitive() && jsonObject2.get("value").getAsJsonPrimitive().isNumber()) {
                        pn2.a(jsonObject2.getAsJsonPrimitive("value").getAsInt());
                    }
                    if (jsonObject2.has("progress") && ph2.l() != null) {
                        try {
                            Constructor constructor = ph2.l().getConstructor(new Class[0]);
                            po po2 = (po)constructor.newInstance(new Object[0]);
                            po2.a(jsonObject2.get("progress"));
                            pn2.a(po2);
                        }
                        catch (Throwable throwable) {
                            b.warn("Invalid statistic progress in " + this.d, throwable);
                        }
                    }
                }
                hashMap.put(ph2, pn2);
                continue;
            }
            b.warn("Invalid statistic in " + this.d + ": Don't know what " + entry.getKey() + " is");
        }
        return hashMap;
    }

    public static String a(Map map) {
        JsonObject jsonObject = new JsonObject();
        for (Map.Entry entry : map.entrySet()) {
            if (((pn)entry.getValue()).b() != null) {
                JsonObject jsonObject2 = new JsonObject();
                jsonObject2.addProperty("value", ((pn)entry.getValue()).a());
                try {
                    jsonObject2.add("progress", ((pn)entry.getValue()).b().a());
                }
                catch (Throwable throwable) {
                    b.warn("Couldn't save statistic " + ((ph)entry.getKey()).e() + ": error serializing progress", throwable);
                }
                jsonObject.add(((ph)entry.getKey()).e, jsonObject2);
                continue;
            }
            jsonObject.addProperty(((ph)entry.getKey()).e, ((pn)entry.getValue()).a());
        }
        return jsonObject.toString();
    }

    public void d() {
        for (ph ph2 : this.a.keySet()) {
            this.e.add(ph2);
        }
    }

    public void a(mw mw2) {
        int n2 = this.c.al();
        HashMap<ph, Integer> hashMap = Maps.newHashMap();
        if (this.g || n2 - this.f > 300) {
            this.f = n2;
            for (ph ph2 : this.c()) {
                hashMap.put(ph2, this.a(ph2));
            }
        }
        mw2.a.a(new gd(hashMap));
    }

    public void b(mw mw2) {
        HashMap<pb, Integer> hashMap = Maps.newHashMap();
        for (pb pb2 : pc.e) {
            if (!this.a(pb2)) continue;
            hashMap.put(pb2, this.a((ph)pb2));
            this.e.remove(pb2);
        }
        mw2.a.a(new gd(hashMap));
    }

    public boolean e() {
        return this.g;
    }
}

