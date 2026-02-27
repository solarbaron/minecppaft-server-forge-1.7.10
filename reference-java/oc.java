/*
 * Decompiled with CFR 0.152.
 */
import com.mojang.authlib.GameProfile;
import com.mojang.authlib.ProfileLookupCallback;
import com.mojang.authlib.yggdrasil.ProfileNotFoundException;
import net.minecraft.server.MinecraftServer;

final class oc
implements ProfileLookupCallback {
    final /* synthetic */ MinecraftServer a;
    final /* synthetic */ oj b;

    oc(MinecraftServer minecraftServer, oj oj2) {
        this.a = minecraftServer;
        this.b = oj2;
    }

    @Override
    public void onProfileLookupSucceeded(GameProfile gameProfile) {
        this.a.ax().a(gameProfile);
        this.b.a(new ok(gameProfile, this.a.l()));
    }

    @Override
    public void onProfileLookupFailed(GameProfile gameProfile, Exception exception) {
        nz.a().warn("Could not lookup oplist entry for " + gameProfile.getName(), (Throwable)exception);
        if (!(exception instanceof ProfileNotFoundException)) {
            throw new oh("Could not request user " + gameProfile.getName() + " from backend systems", exception, null);
        }
    }
}

