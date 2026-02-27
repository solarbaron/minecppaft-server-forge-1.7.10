/*
 * Decompiled with CFR 0.152.
 */
import com.google.common.base.Charsets;
import com.google.common.collect.Iterators;
import com.google.common.collect.Lists;
import com.google.common.collect.Maps;
import com.google.common.io.Files;
import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.mojang.authlib.Agent;
import com.mojang.authlib.GameProfile;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.Reader;
import java.lang.reflect.ParameterizedType;
import java.lang.reflect.Type;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import java.util.LinkedList;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.UUID;
import net.minecraft.server.MinecraftServer;
import org.apache.commons.io.IOUtils;

public class ns {
    public static final SimpleDateFormat a = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss Z");
    private final Map c = Maps.newHashMap();
    private final Map d = Maps.newHashMap();
    private final LinkedList e = Lists.newLinkedList();
    private final MinecraftServer f;
    protected final Gson b;
    private final File g;
    private static final ParameterizedType h = new nu();

    public ns(MinecraftServer minecraftServer, File file) {
        this.f = minecraftServer;
        this.g = file;
        GsonBuilder gsonBuilder = new GsonBuilder();
        gsonBuilder.registerTypeHierarchyAdapter(nv.class, new nw(this, null));
        this.b = gsonBuilder.create();
        this.b();
    }

    private static GameProfile a(MinecraftServer minecraftServer, String string) {
        GameProfile[] gameProfileArray = new GameProfile[1];
        nt nt2 = new nt(gameProfileArray);
        minecraftServer.aw().findProfilesByNames(new String[]{string}, Agent.MINECRAFT, nt2);
        if (!minecraftServer.Y() && gameProfileArray[0] == null) {
            UUID uUID = yz.a(new GameProfile(null, string));
            GameProfile gameProfile = new GameProfile(uUID, string);
            nt2.onProfileLookupSucceeded(gameProfile);
        }
        return gameProfileArray[0];
    }

    public void a(GameProfile gameProfile) {
        this.a(gameProfile, null);
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    private void a(GameProfile gameProfile, Date date) {
        Object object;
        UUID uUID = gameProfile.getId();
        if (date == null) {
            object = Calendar.getInstance();
            ((Calendar)object).setTime(new Date());
            ((Calendar)object).add(2, 1);
            date = ((Calendar)object).getTime();
        }
        object = gameProfile.getName().toLowerCase(Locale.ROOT);
        nv nv2 = new nv(this, gameProfile, date, null);
        LinkedList linkedList = this.e;
        synchronized (linkedList) {
            if (this.d.containsKey(uUID)) {
                nv nv3 = (nv)this.d.get(uUID);
                this.c.remove(nv3.a().getName().toLowerCase(Locale.ROOT));
                this.c.put(gameProfile.getName().toLowerCase(Locale.ROOT), nv2);
                this.e.remove(gameProfile);
            } else {
                this.d.put(uUID, nv2);
                this.c.put(object, nv2);
            }
            this.e.addFirst(gameProfile);
        }
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    public GameProfile a(String string) {
        Object object;
        String string2 = string.toLowerCase(Locale.ROOT);
        nv nv2 = (nv)this.c.get(string2);
        if (nv2 != null && new Date().getTime() >= nv.a(nv2).getTime()) {
            this.d.remove(nv2.a().getId());
            this.c.remove(nv2.a().getName().toLowerCase(Locale.ROOT));
            object = this.e;
            synchronized (object) {
                this.e.remove(nv2.a());
            }
            nv2 = null;
        }
        if (nv2 != null) {
            object = nv2.a();
            LinkedList linkedList = this.e;
            synchronized (linkedList) {
                this.e.remove(object);
                this.e.addFirst(object);
            }
        } else {
            object = ns.a(this.f, string2);
            if (object != null) {
                this.a((GameProfile)object);
                nv2 = (nv)this.c.get(string2);
            }
        }
        this.c();
        return nv2 == null ? null : nv2.a();
    }

    public String[] a() {
        ArrayList arrayList = Lists.newArrayList(this.c.keySet());
        return arrayList.toArray(new String[arrayList.size()]);
    }

    public GameProfile a(UUID uUID) {
        nv nv2 = (nv)this.d.get(uUID);
        return nv2 == null ? null : nv2.a();
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    private nv b(UUID uUID) {
        nv nv2 = (nv)this.d.get(uUID);
        if (nv2 != null) {
            GameProfile gameProfile = nv2.a();
            LinkedList linkedList = this.e;
            synchronized (linkedList) {
                this.e.remove(gameProfile);
                this.e.addFirst(gameProfile);
            }
        }
        return nv2;
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     * Iterators could be improved
     * Loose catch block
     */
    public void b() {
        List list = null;
        BufferedReader bufferedReader = null;
        try {
            bufferedReader = Files.newReader(this.g, Charsets.UTF_8);
            list = (List)this.b.fromJson((Reader)bufferedReader, (Type)h);
        }
        catch (FileNotFoundException fileNotFoundException) {
            IOUtils.closeQuietly(bufferedReader);
            return;
            catch (Throwable throwable) {
                IOUtils.closeQuietly(bufferedReader);
                throw throwable;
            }
        }
        IOUtils.closeQuietly(bufferedReader);
        if (list != null) {
            this.c.clear();
            this.d.clear();
            LinkedList linkedList = this.e;
            synchronized (linkedList) {
                this.e.clear();
            }
            list = Lists.reverse(list);
            for (nv nv2 : list) {
                if (nv2 == null) continue;
                this.a(nv2.a(), nv2.b());
            }
        }
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     * Loose catch block
     */
    public void c() {
        String string = this.b.toJson(this.a(1000));
        BufferedWriter bufferedWriter = null;
        try {
            bufferedWriter = Files.newWriter(this.g, Charsets.UTF_8);
            bufferedWriter.write(string);
        }
        catch (FileNotFoundException fileNotFoundException) {
            IOUtils.closeQuietly(bufferedWriter);
            return;
        }
        catch (IOException iOException) {
            IOUtils.closeQuietly(bufferedWriter);
            return;
            {
                catch (Throwable throwable) {
                    IOUtils.closeQuietly(bufferedWriter);
                    throw throwable;
                }
            }
        }
        IOUtils.closeQuietly(bufferedWriter);
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    private List a(int n2) {
        ArrayList<GameProfile> arrayList;
        ArrayList<nv> arrayList2 = Lists.newArrayList();
        LinkedList linkedList = this.e;
        synchronized (linkedList) {
            arrayList = Lists.newArrayList(Iterators.limit(this.e.iterator(), n2));
        }
        for (GameProfile gameProfile : arrayList) {
            nv nv2 = this.b(gameProfile.getId());
            if (nv2 == null) continue;
            arrayList2.add(nv2);
        }
        return arrayList2;
    }
}

