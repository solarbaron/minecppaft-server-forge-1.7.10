/*
 * Decompiled with CFR 0.152.
 */
import com.mojang.authlib.GameProfile;
import com.mojang.authlib.ProfileLookupCallback;

final class nt
implements ProfileLookupCallback {
    final /* synthetic */ GameProfile[] a;

    nt(GameProfile[] gameProfileArray) {
        this.a = gameProfileArray;
    }

    @Override
    public void onProfileLookupSucceeded(GameProfile gameProfile) {
        this.a[0] = gameProfile;
    }

    @Override
    public void onProfileLookupFailed(GameProfile gameProfile, Exception exception) {
        this.a[0] = null;
    }
}

