/*
 * Decompiled with CFR 0.152.
 */
import com.mojang.authlib.GameProfile;
import com.mojang.authlib.ProfileLookupCallback;
import com.mojang.authlib.yggdrasil.ProfileNotFoundException;
import java.util.Date;
import java.util.Locale;
import java.util.Map;
import net.minecraft.server.MinecraftServer;

final class ob
implements ProfileLookupCallback {
    final /* synthetic */ MinecraftServer a;
    final /* synthetic */ Map b;
    final /* synthetic */ op c;

    ob(MinecraftServer minecraftServer, Map map, op op2) {
        this.a = minecraftServer;
        this.b = map;
        this.c = op2;
    }

    @Override
    public void onProfileLookupSucceeded(GameProfile gameProfile) {
        this.a.ax().a(gameProfile);
        String[] stringArray = (String[])this.b.get(gameProfile.getName().toLowerCase(Locale.ROOT));
        if (stringArray == null) {
            nz.a().warn("Could not convert user banlist entry for " + gameProfile.getName());
            throw new oh("Profile not in the conversionlist", null);
        }
        Date date = stringArray.length > 1 ? nz.a(stringArray[1], null) : null;
        String string = stringArray.length > 2 ? stringArray[2] : null;
        Date date2 = stringArray.length > 3 ? nz.a(stringArray[3], null) : null;
        String string2 = stringArray.length > 4 ? stringArray[4] : null;
        this.c.a(new oq(gameProfile, date, string, date2, string2));
    }

    @Override
    public void onProfileLookupFailed(GameProfile gameProfile, Exception exception) {
        nz.a().warn("Could not lookup user banlist entry for " + gameProfile.getName(), (Throwable)exception);
        if (!(exception instanceof ProfileNotFoundException)) {
            throw new oh("Could not request user " + gameProfile.getName() + " from backend systems", exception, null);
        }
    }
}

