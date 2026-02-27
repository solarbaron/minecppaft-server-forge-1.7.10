/*
 * Decompiled with CFR 0.152.
 */
import com.google.common.util.concurrent.ThreadFactoryBuilder;
import io.netty.bootstrap.ServerBootstrap;
import io.netty.channel.ChannelFuture;
import io.netty.channel.nio.NioEventLoopGroup;
import io.netty.channel.socket.nio.NioServerSocketChannel;
import java.net.InetAddress;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;
import net.minecraft.server.MinecraftServer;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class nc {
    private static final Logger b = LogManager.getLogger();
    private static final NioEventLoopGroup c = new NioEventLoopGroup(0, new ThreadFactoryBuilder().setNameFormat("Netty IO #%d").setDaemon(true).build());
    private final MinecraftServer d;
    public volatile boolean a;
    private final List e = Collections.synchronizedList(new ArrayList());
    private final List f = Collections.synchronizedList(new ArrayList());

    public nc(MinecraftServer minecraftServer) {
        this.d = minecraftServer;
        this.a = true;
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    public void a(InetAddress inetAddress, int n2) {
        List list = this.e;
        synchronized (list) {
            this.e.add(((ServerBootstrap)((ServerBootstrap)new ServerBootstrap().channel(NioServerSocketChannel.class)).childHandler(new nd(this)).group(c).localAddress(inetAddress, n2)).bind().syncUninterruptibly());
        }
    }

    public void b() {
        this.a = false;
        for (ChannelFuture channelFuture : this.e) {
            channelFuture.channel().close().syncUninterruptibly();
        }
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    public void c() {
        List list = this.f;
        synchronized (list) {
            Iterator iterator = this.f.iterator();
            while (iterator.hasNext()) {
                ej ej2 = (ej)iterator.next();
                if (!ej2.d()) {
                    iterator.remove();
                    if (ej2.f() != null) {
                        ej2.e().a(ej2.f());
                        continue;
                    }
                    if (ej2.e() == null) continue;
                    ej2.e().a(new fq("Disconnected"));
                    continue;
                }
                try {
                    ej2.a();
                }
                catch (Exception exception) {
                    Object object;
                    if (ej2.c()) {
                        object = b.a(exception, "Ticking memory connection");
                        k k2 = ((b)object).a("Ticking connection");
                        k2.a("Connection", new nf(this, ej2));
                        throw new s((b)object);
                    }
                    b.warn("Failed to handle packet for " + ej2.b(), (Throwable)exception);
                    object = new fq("Internal server error");
                    ej2.a(new gs((fj)object), new ng(this, ej2, (fq)object));
                    ej2.g();
                }
            }
        }
    }

    public MinecraftServer d() {
        return this.d;
    }

    static /* synthetic */ List a(nc nc2) {
        return nc2.f;
    }

    static /* synthetic */ MinecraftServer b(nc nc2) {
        return nc2.d;
    }
}

