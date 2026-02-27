/*
 * Decompiled with CFR 0.152.
 */
import com.mojang.authlib.GameProfile;
import com.mojang.authlib.ProfileLookupCallback;
import com.mojang.authlib.yggdrasil.ProfileNotFoundException;
import java.io.File;
import java.util.UUID;

final class of
implements ProfileLookupCallback {
    final /* synthetic */ lt a;
    final /* synthetic */ File b;
    final /* synthetic */ File c;
    final /* synthetic */ File d;
    final /* synthetic */ String[] e;

    of(lt lt2, File file, File file2, File file3, String[] stringArray) {
        this.a = lt2;
        this.b = file;
        this.c = file2;
        this.d = file3;
        this.e = stringArray;
    }

    @Override
    public void onProfileLookupSucceeded(GameProfile gameProfile) {
        this.a.ax().a(gameProfile);
        UUID uUID = gameProfile.getId();
        if (uUID == null) {
            throw new oh("Missing UUID for user profile " + gameProfile.getName(), null);
        }
        this.a(this.b, this.a(gameProfile), uUID.toString());
    }

    @Override
    public void onProfileLookupFailed(GameProfile gameProfile, Exception exception) {
        nz.a().warn("Could not lookup user uuid for " + gameProfile.getName(), (Throwable)exception);
        if (!(exception instanceof ProfileNotFoundException)) {
            throw new oh("Could not request user " + gameProfile.getName() + " from backend systems", exception, null);
        }
        String string = this.a(gameProfile);
        this.a(this.c, string, string);
    }

    private void a(File file, String string, String string2) {
        File file2 = new File(this.d, string + ".dat");
        File file3 = new File(file, string2 + ".dat");
        nz.a(file);
        if (!file2.renameTo(file3)) {
            throw new oh("Could not convert file for " + string, null);
        }
    }

    private String a(GameProfile gameProfile) {
        String string = null;
        for (int i2 = 0; i2 < this.e.length; ++i2) {
            if (this.e[i2] == null || !this.e[i2].equalsIgnoreCase(gameProfile.getName())) continue;
            string = this.e[i2];
            break;
        }
        if (string == null) {
            throw new oh("Could not find the filename for " + gameProfile.getName() + " anymore", null);
        }
        return string;
    }
}

