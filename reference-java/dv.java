/*
 * Decompiled with CFR 0.152.
 */
import com.mojang.authlib.GameProfile;
import com.mojang.authlib.properties.Property;
import java.util.UUID;

public final class dv {
    public static GameProfile a(dh dh2) {
        String string = null;
        String string2 = null;
        if (dh2.b("Name", 8)) {
            string = dh2.j("Name");
        }
        if (dh2.b("Id", 8)) {
            string2 = dh2.j("Id");
        }
        if (!qn.b(string) || !qn.b(string2)) {
            UUID uUID;
            try {
                uUID = UUID.fromString(string2);
            }
            catch (Throwable throwable) {
                uUID = null;
            }
            GameProfile gameProfile = new GameProfile(uUID, string);
            if (dh2.b("Properties", 10)) {
                dh dh3 = dh2.m("Properties");
                for (String string3 : dh3.c()) {
                    dq dq2 = dh3.c(string3, 10);
                    for (int i2 = 0; i2 < dq2.c(); ++i2) {
                        dh dh4 = dq2.b(i2);
                        String string4 = dh4.j("Value");
                        if (dh4.b("Signature", 8)) {
                            gameProfile.getProperties().put(string3, new Property(string3, string4, dh4.j("Signature")));
                            continue;
                        }
                        gameProfile.getProperties().put(string3, new Property(string3, string4));
                    }
                }
            }
            return gameProfile;
        }
        return null;
    }

    public static void a(dh dh2, GameProfile gameProfile) {
        if (!qn.b(gameProfile.getName())) {
            dh2.a("Name", gameProfile.getName());
        }
        if (gameProfile.getId() != null) {
            dh2.a("Id", gameProfile.getId().toString());
        }
        if (!gameProfile.getProperties().isEmpty()) {
            dh dh3 = new dh();
            for (String string : gameProfile.getProperties().keySet()) {
                dq dq2 = new dq();
                for (Property property : gameProfile.getProperties().get(string)) {
                    dh dh4 = new dh();
                    dh4.a("Value", property.getValue());
                    if (property.hasSignature()) {
                        dh4.a("Signature", property.getSignature());
                    }
                    dq2.a(dh4);
                }
                dh3.a(string, dq2);
            }
            dh2.a("Properties", dh3);
        }
    }
}

