/*
 * Decompiled with CFR 0.152.
 */
import com.google.common.base.Charsets;
import com.mojang.authlib.GameProfile;
import io.netty.util.concurrent.GenericFutureListener;
import java.security.PrivateKey;
import java.util.Arrays;
import java.util.Random;
import java.util.UUID;
import java.util.concurrent.atomic.AtomicInteger;
import javax.crypto.SecretKey;
import net.minecraft.server.MinecraftServer;
import org.apache.commons.lang3.Validate;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class nn
implements jv {
    private static final AtomicInteger b = new AtomicInteger(0);
    private static final Logger c = LogManager.getLogger();
    private static final Random d = new Random();
    private final byte[] e = new byte[4];
    private final MinecraftServer f;
    public final ej a;
    private np g = np.a;
    private int h;
    private GameProfile i;
    private String j = "";
    private SecretKey k;

    public nn(MinecraftServer minecraftServer, ej ej2) {
        this.f = minecraftServer;
        this.a = ej2;
        d.nextBytes(this.e);
    }

    @Override
    public void a() {
        if (this.g == np.d) {
            this.c();
        }
        if (this.h++ == 600) {
            this.a("Took too long to log in");
        }
    }

    public void a(String string) {
        try {
            c.info("Disconnecting " + this.d() + ": " + string);
            fq fq2 = new fq(string);
            this.a.a(new ju(fq2), new GenericFutureListener[0]);
            this.a.a(fq2);
        }
        catch (Exception exception) {
            c.error("Error whilst disconnecting player", (Throwable)exception);
        }
    }

    public void c() {
        String string;
        if (!this.i.isComplete()) {
            this.i = this.a(this.i);
        }
        if ((string = this.f.ah().a(this.a.b(), this.i)) != null) {
            this.a(string);
        } else {
            this.g = np.e;
            this.a.a(new js(this.i), new GenericFutureListener[0]);
            this.f.ah().a(this.a, this.f.ah().f(this.i));
        }
    }

    @Override
    public void a(fj fj2) {
        c.info(this.d() + " lost connection: " + fj2.c());
    }

    public String d() {
        if (this.i != null) {
            return this.i.toString() + " (" + this.a.b().toString() + ")";
        }
        return String.valueOf(this.a.b());
    }

    @Override
    public void a(eo eo2, eo eo3) {
        Validate.validState(this.g == np.e || this.g == np.a, "Unexpected change in protocol", new Object[0]);
        Validate.validState(eo3 == eo.b || eo3 == eo.d, "Unexpected protocol " + (Object)((Object)eo3), new Object[0]);
    }

    @Override
    public void a(jw jw2) {
        Validate.validState(this.g == np.a, "Unexpected hello packet", new Object[0]);
        this.i = jw2.c();
        if (this.f.Y() && !this.a.c()) {
            this.g = np.b;
            this.a.a(new jt(this.j, this.f.K().getPublic(), this.e), new GenericFutureListener[0]);
        } else {
            this.g = np.d;
        }
    }

    @Override
    public void a(jx jx2) {
        Validate.validState(this.g == np.b, "Unexpected key packet", new Object[0]);
        PrivateKey privateKey = this.f.K().getPrivate();
        if (!Arrays.equals(this.e, jx2.b(privateKey))) {
            throw new IllegalStateException("Invalid nonce!");
        }
        this.k = jx2.a(privateKey);
        this.g = np.c;
        this.a.a(this.k);
        new no(this, "User Authenticator #" + b.incrementAndGet()).start();
    }

    protected GameProfile a(GameProfile gameProfile) {
        UUID uUID = UUID.nameUUIDFromBytes(("OfflinePlayer:" + gameProfile.getName()).getBytes(Charsets.UTF_8));
        return new GameProfile(uUID, gameProfile.getName());
    }

    static /* synthetic */ GameProfile a(nn nn2) {
        return nn2.i;
    }

    static /* synthetic */ String b(nn nn2) {
        return nn2.j;
    }

    static /* synthetic */ MinecraftServer c(nn nn2) {
        return nn2.f;
    }

    static /* synthetic */ SecretKey d(nn nn2) {
        return nn2.k;
    }

    static /* synthetic */ GameProfile a(nn nn2, GameProfile gameProfile) {
        nn2.i = gameProfile;
        return nn2.i;
    }

    static /* synthetic */ Logger e() {
        return c;
    }

    static /* synthetic */ np a(nn nn2, np np2) {
        nn2.g = np2;
        return nn2.g;
    }
}

