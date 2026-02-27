/*
 * Decompiled with CFR 0.152.
 */
import com.mojang.authlib.GameProfile;
import com.mojang.authlib.ProfileLookupCallback;
import com.mojang.authlib.yggdrasil.ProfileNotFoundException;
import net.minecraft.server.MinecraftServer;

final class od
implements ProfileLookupCallback {
    final /* synthetic */ MinecraftServer a;
    final /* synthetic */ or b;

    od(MinecraftServer minecraftServer, or or2) {
        this.a = minecraftServer;
        this.b = or2;
    }

    @Override
    public void onProfileLookupSucceeded(GameProfile gameProfile) {
        this.a.ax().a(gameProfile);
        this.b.a(new os(gameProfile));
    }

    @Override
    public void onProfileLookupFailed(GameProfile gameProfile, Exception exception) {
        nz.a().warn("Could not lookup user whitelist entry for " + gameProfile.getName(), (Throwable)exception);
        if (!(exception instanceof ProfileNotFoundException)) {
            throw new oh("Could not request user " + gameProfile.getName() + " from backend systems", exception, null);
        }
    }
}

