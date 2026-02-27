/*
 * Decompiled with CFR 0.152.
 */
import com.google.gson.JsonObject;
import com.mojang.authlib.GameProfile;
import java.io.File;

public class oj
extends om {
    public oj(File file) {
        super(file);
    }

    @Override
    protected ol a(JsonObject jsonObject) {
        return new ok(jsonObject);
    }

    @Override
    public String[] a() {
        String[] stringArray = new String[this.e().size()];
        int n2 = 0;
        for (ok ok2 : this.e().values()) {
            stringArray[n2++] = ((GameProfile)ok2.f()).getName();
        }
        return stringArray;
    }

    protected String b(GameProfile gameProfile) {
        return gameProfile.getId().toString();
    }

    public GameProfile a(String string) {
        for (ok ok2 : this.e().values()) {
            if (!string.equalsIgnoreCase(((GameProfile)ok2.f()).getName())) continue;
            return (GameProfile)ok2.f();
        }
        return null;
    }

    @Override
    protected /* synthetic */ String a(Object object) {
        return this.b((GameProfile)object);
    }
}

