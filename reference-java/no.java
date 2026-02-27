/*
 * Decompiled with CFR 0.152.
 */
import com.mojang.authlib.GameProfile;
import com.mojang.authlib.exceptions.AuthenticationUnavailableException;
import java.math.BigInteger;

class no
extends Thread {
    final /* synthetic */ nn a;

    no(nn nn2, String string) {
        this.a = nn2;
        super(string);
    }

    @Override
    public void run() {
        GameProfile gameProfile = nn.a(this.a);
        try {
            String string = new BigInteger(pt.a(nn.b(this.a), nn.c(this.a).K().getPublic(), nn.d(this.a))).toString(16);
            nn.a(this.a, nn.c(this.a).av().hasJoinedServer(new GameProfile(null, gameProfile.getName()), string));
            if (nn.a(this.a) != null) {
                nn.e().info("UUID of player " + nn.a(this.a).getName() + " is " + nn.a(this.a).getId());
                nn.a(this.a, np.d);
            } else if (nn.c(this.a).N()) {
                nn.e().warn("Failed to verify username but will let them in anyway!");
                nn.a(this.a, this.a.a(gameProfile));
                nn.a(this.a, np.d);
            } else {
                this.a.a("Failed to verify username!");
                nn.e().error("Username '" + nn.a(this.a).getName() + "' tried to join with an invalid session");
            }
        }
        catch (AuthenticationUnavailableException authenticationUnavailableException) {
            if (nn.c(this.a).N()) {
                nn.e().warn("Authentication servers are down but will let them in anyway!");
                nn.a(this.a, this.a.a(gameProfile));
                nn.a(this.a, np.d);
            }
            this.a.a("Authentication servers are down. Please try again later, sorry!");
            nn.e().error("Couldn't verify username because servers are unavailable");
        }
    }
}

