/*
 * Decompiled with CFR 0.152.
 */
import com.google.common.base.Charsets;
import com.google.common.collect.Iterators;
import com.google.common.collect.Lists;
import com.google.common.collect.Maps;
import com.google.common.io.Files;
import com.mojang.authlib.Agent;
import com.mojang.authlib.GameProfile;
import com.mojang.authlib.ProfileLookupCallback;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.text.ParseException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.UUID;
import net.minecraft.server.MinecraftServer;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class nz {
    private static final Logger e = LogManager.getLogger();
    public static final File a = new File("banned-ips.txt");
    public static final File b = new File("banned-players.txt");
    public static final File c = new File("ops.txt");
    public static final File d = new File("white-list.txt");

    static List a(File file, Map map) {
        List<String> list = Files.readLines(file, Charsets.UTF_8);
        for (String string : list) {
            if ((string = string.trim()).startsWith("#") || string.length() < 1) continue;
            String[] stringArray = string.split("\\|");
            map.put(stringArray[0].toLowerCase(Locale.ROOT), stringArray);
        }
        return list;
    }

    private static void a(MinecraftServer minecraftServer, Collection collection, ProfileLookupCallback profileLookupCallback) {
        String[] stringArray = Iterators.toArray(Iterators.filter(collection.iterator(), new oa()), String.class);
        if (minecraftServer.Y()) {
            minecraftServer.aw().findProfilesByNames(stringArray, Agent.MINECRAFT, profileLookupCallback);
        } else {
            for (String string : stringArray) {
                UUID uUID = yz.a(new GameProfile(null, string));
                GameProfile gameProfile = new GameProfile(uUID, string);
                profileLookupCallback.onProfileLookupSucceeded(gameProfile);
            }
        }
    }

    public static boolean a(MinecraftServer minecraftServer) {
        op op2 = new op(oi.a);
        if (b.exists() && b.isFile()) {
            if (op2.c().exists()) {
                try {
                    op2.g();
                }
                catch (FileNotFoundException fileNotFoundException) {
                    e.warn("Could not load existing file " + op2.c().getName(), (Throwable)fileNotFoundException);
                }
            }
            try {
                HashMap hashMap = Maps.newHashMap();
                nz.a(b, hashMap);
                ob ob2 = new ob(minecraftServer, hashMap, op2);
                nz.a(minecraftServer, hashMap.keySet(), ob2);
                op2.f();
                nz.c(b);
            }
            catch (IOException iOException) {
                e.warn("Could not read old user banlist to convert it!", (Throwable)iOException);
                return false;
            }
            catch (oh oh2) {
                e.error("Conversion failed, please try again later", (Throwable)oh2);
                return false;
            }
            return true;
        }
        return true;
    }

    public static boolean b(MinecraftServer minecraftServer) {
        nx nx2 = new nx(oi.b);
        if (a.exists() && a.isFile()) {
            if (nx2.c().exists()) {
                try {
                    nx2.g();
                }
                catch (FileNotFoundException fileNotFoundException) {
                    e.warn("Could not load existing file " + nx2.c().getName(), (Throwable)fileNotFoundException);
                }
            }
            try {
                HashMap hashMap = Maps.newHashMap();
                nz.a(a, hashMap);
                for (String string : hashMap.keySet()) {
                    String[] stringArray = (String[])hashMap.get(string);
                    Date date = stringArray.length > 1 ? nz.b(stringArray[1], null) : null;
                    String string2 = stringArray.length > 2 ? stringArray[2] : null;
                    Date date2 = stringArray.length > 3 ? nz.b(stringArray[3], null) : null;
                    String string3 = stringArray.length > 4 ? stringArray[4] : null;
                    nx2.a(new ny(string, date, string2, date2, string3));
                }
                nx2.f();
                nz.c(a);
            }
            catch (IOException iOException) {
                e.warn("Could not parse old ip banlist to convert it!", (Throwable)iOException);
                return false;
            }
            return true;
        }
        return true;
    }

    public static boolean c(MinecraftServer minecraftServer) {
        oj oj2 = new oj(oi.c);
        if (c.exists() && c.isFile()) {
            if (oj2.c().exists()) {
                try {
                    oj2.g();
                }
                catch (FileNotFoundException fileNotFoundException) {
                    e.warn("Could not load existing file " + oj2.c().getName(), (Throwable)fileNotFoundException);
                }
            }
            try {
                List<String> list = Files.readLines(c, Charsets.UTF_8);
                oc oc2 = new oc(minecraftServer, oj2);
                nz.a(minecraftServer, list, oc2);
                oj2.f();
                nz.c(c);
            }
            catch (IOException iOException) {
                e.warn("Could not read old oplist to convert it!", (Throwable)iOException);
                return false;
            }
            catch (oh oh2) {
                e.error("Conversion failed, please try again later", (Throwable)oh2);
                return false;
            }
            return true;
        }
        return true;
    }

    public static boolean d(MinecraftServer minecraftServer) {
        or or2 = new or(oi.d);
        if (d.exists() && d.isFile()) {
            if (or2.c().exists()) {
                try {
                    or2.g();
                }
                catch (FileNotFoundException fileNotFoundException) {
                    e.warn("Could not load existing file " + or2.c().getName(), (Throwable)fileNotFoundException);
                }
            }
            try {
                List<String> list = Files.readLines(d, Charsets.UTF_8);
                od od2 = new od(minecraftServer, or2);
                nz.a(minecraftServer, list, od2);
                or2.f();
                nz.c(d);
            }
            catch (IOException iOException) {
                e.warn("Could not read old whitelist to convert it!", (Throwable)iOException);
                return false;
            }
            catch (oh oh2) {
                e.error("Conversion failed, please try again later", (Throwable)oh2);
                return false;
            }
            return true;
        }
        return true;
    }

    public static String a(String string) {
        if (qn.b(string) || string.length() > 16) {
            return string;
        }
        MinecraftServer minecraftServer = MinecraftServer.I();
        GameProfile gameProfile = minecraftServer.ax().a(string);
        if (gameProfile != null && gameProfile.getId() != null) {
            return gameProfile.getId().toString();
        }
        if (minecraftServer.N() || !minecraftServer.Y()) {
            return yz.a(new GameProfile(null, string)).toString();
        }
        ArrayList arrayList = Lists.newArrayList();
        oe oe2 = new oe(minecraftServer, arrayList);
        nz.a(minecraftServer, Lists.newArrayList(string), oe2);
        if (arrayList.size() > 0 && ((GameProfile)arrayList.get(0)).getId() != null) {
            return ((GameProfile)arrayList.get(0)).getId().toString();
        }
        return "";
    }

    public static boolean a(lt lt2, lq lq2) {
        File file = nz.d(lq2);
        File file2 = new File(file.getParentFile(), "playerdata");
        File file3 = new File(file.getParentFile(), "unknownplayers");
        if (!file.exists() || !file.isDirectory()) {
            return true;
        }
        File[] fileArray = file.listFiles();
        ArrayList<String> arrayList = Lists.newArrayList();
        for (File file4 : fileArray) {
            String string;
            String string2 = file4.getName();
            if (!string2.toLowerCase(Locale.ROOT).endsWith(".dat") || (string = string2.substring(0, string2.length() - ".dat".length())).length() <= 0) continue;
            arrayList.add(string);
        }
        try {
            Object[] objectArray = arrayList.toArray(new String[arrayList.size()]);
            of of2 = new of(lt2, file2, file3, file, (String[])objectArray);
            nz.a(lt2, Lists.newArrayList(objectArray), of2);
        }
        catch (oh oh2) {
            e.error("Conversion failed, please try again later", (Throwable)oh2);
            return false;
        }
        return true;
    }

    private static void b(File file) {
        if (file.exists()) {
            if (file.isDirectory()) {
                return;
            }
            throw new oh("Can't create directory " + file.getName() + " in world save directory.", null);
        }
        if (!file.mkdirs()) {
            throw new oh("Can't create directory " + file.getName() + " in world save directory.", null);
        }
    }

    public static boolean a(lq lq2) {
        boolean bl2 = nz.b(lq2);
        bl2 = bl2 && nz.c(lq2);
        return bl2;
    }

    private static boolean b(lq lq2) {
        boolean bl2 = false;
        if (b.exists() && b.isFile()) {
            bl2 = true;
        }
        boolean bl3 = false;
        if (a.exists() && a.isFile()) {
            bl3 = true;
        }
        boolean bl4 = false;
        if (c.exists() && c.isFile()) {
            bl4 = true;
        }
        boolean bl5 = false;
        if (d.exists() && d.isFile()) {
            bl5 = true;
        }
        if (bl2 || bl3 || bl4 || bl5) {
            e.warn("**** FAILED TO START THE SERVER AFTER ACCOUNT CONVERSION!");
            e.warn("** please remove the following files and restart the server:");
            if (bl2) {
                e.warn("* " + b.getName());
            }
            if (bl3) {
                e.warn("* " + a.getName());
            }
            if (bl4) {
                e.warn("* " + c.getName());
            }
            if (bl5) {
                e.warn("* " + d.getName());
            }
            return false;
        }
        return true;
    }

    private static boolean c(lq lq2) {
        String[] stringArray;
        File file = nz.d(lq2);
        if (file.exists() && file.isDirectory() && (stringArray = file.list(new og())).length > 0) {
            e.warn("**** DETECTED OLD PLAYER FILES IN THE WORLD SAVE");
            e.warn("**** THIS USUALLY HAPPENS WHEN THE AUTOMATIC CONVERSION FAILED IN SOME WAY");
            e.warn("** please restart the server and if the problem persists, remove the directory '{}'", file.getPath());
            return false;
        }
        return true;
    }

    private static File d(lq lq2) {
        String string = lq2.a("level-name", "world");
        File file = new File(string);
        return new File(file, "players");
    }

    private static void c(File file) {
        File file2 = new File(file.getName() + ".converted");
        file.renameTo(file2);
    }

    private static Date b(String string, Date date) {
        Date date2;
        try {
            date2 = nr.a.parse(string);
        }
        catch (ParseException parseException) {
            date2 = date;
        }
        return date2;
    }

    static /* synthetic */ Logger a() {
        return e;
    }

    static /* synthetic */ Date a(String string, Date date) {
        return nz.b(string, date);
    }

    static /* synthetic */ void a(File file) {
        nz.b(file);
    }
}

