/*
 * Decompiled with CFR 0.152.
 */
import com.mojang.authlib.GameProfile;
import java.io.IOException;
import net.minecraft.server.MinecraftServer;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class ls
extends oi {
    private static final Logger g = LogManager.getLogger();

    public ls(lt lt2) {
        super(lt2);
        this.a(lt2.a("view-distance", 10));
        this.f = lt2.a("max-players", 20);
        this.a(lt2.a("white-list", false));
        if (!lt2.N()) {
            this.h().a(true);
            this.i().a(true);
        }
        this.y();
        this.w();
        this.x();
        this.v();
        this.z();
        this.B();
        this.A();
        if (!this.k().c().exists()) {
            this.C();
        }
    }

    @Override
    public void a(boolean bl2) {
        super.a(bl2);
        this.b().a("white-list", (Object)bl2);
        this.b().a();
    }

    @Override
    public void a(GameProfile gameProfile) {
        super.a(gameProfile);
        this.A();
    }

    @Override
    public void b(GameProfile gameProfile) {
        super.b(gameProfile);
        this.A();
    }

    @Override
    public void c(GameProfile gameProfile) {
        super.c(gameProfile);
        this.C();
    }

    @Override
    public void d(GameProfile gameProfile) {
        super.d(gameProfile);
        this.C();
    }

    @Override
    public void a() {
        this.B();
    }

    private void v() {
        try {
            this.i().f();
        }
        catch (IOException iOException) {
            g.warn("Failed to save ip banlist: ", (Throwable)iOException);
        }
    }

    private void w() {
        try {
            this.h().f();
        }
        catch (IOException iOException) {
            g.warn("Failed to save user banlist: ", (Throwable)iOException);
        }
    }

    private void x() {
        try {
            this.i().g();
        }
        catch (IOException iOException) {
            g.warn("Failed to load ip banlist: ", (Throwable)iOException);
        }
    }

    private void y() {
        try {
            this.h().g();
        }
        catch (IOException iOException) {
            g.warn("Failed to load user banlist: ", (Throwable)iOException);
        }
    }

    private void z() {
        try {
            this.m().g();
        }
        catch (Exception exception) {
            g.warn("Failed to load operators list: ", (Throwable)exception);
        }
    }

    private void A() {
        try {
            this.m().f();
        }
        catch (Exception exception) {
            g.warn("Failed to save operators list: ", (Throwable)exception);
        }
    }

    private void B() {
        try {
            this.k().g();
        }
        catch (Exception exception) {
            g.warn("Failed to load white-list: ", (Throwable)exception);
        }
    }

    private void C() {
        try {
            this.k().f();
        }
        catch (Exception exception) {
            g.warn("Failed to save white-list: ", (Throwable)exception);
        }
    }

    @Override
    public boolean e(GameProfile gameProfile) {
        return !this.r() || this.g(gameProfile) || this.k().a(gameProfile);
    }

    public lt b() {
        return (lt)super.c();
    }

    @Override
    public /* synthetic */ MinecraftServer c() {
        return this.b();
    }
}

