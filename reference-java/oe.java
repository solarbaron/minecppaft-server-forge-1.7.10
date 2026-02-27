/*
 * Decompiled with CFR 0.152.
 */
import com.mojang.authlib.GameProfile;
import com.mojang.authlib.ProfileLookupCallback;
import java.util.List;
import net.minecraft.server.MinecraftServer;

final class oe
implements ProfileLookupCallback {
    final /* synthetic */ MinecraftServer a;
    final /* synthetic */ List b;

    oe(MinecraftServer minecraftServer, List list) {
        this.a = minecraftServer;
        this.b = list;
    }

    @Override
    public void onProfileLookupSucceeded(GameProfile gameProfile) {
        this.a.ax().a(gameProfile);
        this.b.add(gameProfile);
    }

    @Override
    public void onProfileLookupFailed(GameProfile gameProfile, Exception exception) {
        nz.a().warn("Could not lookup user whitelist entry for " + gameProfile.getName(), (Throwable)exception);
    }
}

