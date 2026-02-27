/*
 * Decompiled with CFR 0.152.
 */
import com.google.gson.JsonObject;
import com.mojang.authlib.GameProfile;
import java.io.File;

public class op
extends om {
    public op(File file) {
        super(file);
    }

    @Override
    protected ol a(JsonObject jsonObject) {
        return new oq(jsonObject);
    }

    public boolean a(GameProfile gameProfile) {
        return this.d(gameProfile);
    }

    @Override
    public String[] a() {
        String[] stringArray = new String[this.e().size()];
        int n2 = 0;
        for (oq oq2 : this.e().values()) {
            stringArray[n2++] = ((GameProfile)oq2.f()).getName();
        }
        return stringArray;
    }

    protected String b(GameProfile gameProfile) {
        return gameProfile.getId().toString();
    }

    public GameProfile a(String string) {
        for (oq oq2 : this.e().values()) {
            if (!string.equalsIgnoreCase(((GameProfile)oq2.f()).getName())) continue;
            return (GameProfile)oq2.f();
        }
        return null;
    }

    @Override
    protected /* synthetic */ String a(Object object) {
        return this.b((GameProfile)object);
    }
}

