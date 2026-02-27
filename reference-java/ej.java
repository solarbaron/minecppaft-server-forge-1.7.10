/*
 * Decompiled with CFR 0.152.
 */
import com.google.common.collect.Queues;
import com.google.common.util.concurrent.ThreadFactoryBuilder;
import io.netty.channel.Channel;
import io.netty.channel.ChannelFutureListener;
import io.netty.channel.ChannelHandlerContext;
import io.netty.channel.SimpleChannelInboundHandler;
import io.netty.channel.local.LocalChannel;
import io.netty.channel.local.LocalServerChannel;
import io.netty.channel.nio.NioEventLoopGroup;
import io.netty.handler.timeout.TimeoutException;
import io.netty.util.AttributeKey;
import io.netty.util.concurrent.GenericFutureListener;
import java.net.SocketAddress;
import java.util.Queue;
import javax.crypto.SecretKey;
import org.apache.commons.lang3.Validate;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.apache.logging.log4j.Marker;
import org.apache.logging.log4j.MarkerManager;

public class ej
extends SimpleChannelInboundHandler {
    private static final Logger i = LogManager.getLogger();
    public static final Marker a = MarkerManager.getMarker("NETWORK");
    public static final Marker b = MarkerManager.getMarker("NETWORK_PACKETS", a);
    public static final Marker c = MarkerManager.getMarker("NETWORK_STAT", a);
    public static final AttributeKey d = new AttributeKey("protocol");
    public static final AttributeKey e = new AttributeKey("receivable_packets");
    public static final AttributeKey f = new AttributeKey("sendable_packets");
    public static final NioEventLoopGroup g = new NioEventLoopGroup(0, new ThreadFactoryBuilder().setNameFormat("Netty Client IO #%d").setDaemon(true).build());
    public static final eu h = new eu();
    private final boolean j;
    private final Queue k = Queues.newConcurrentLinkedQueue();
    private final Queue l = Queues.newConcurrentLinkedQueue();
    private Channel m;
    private SocketAddress n;
    private fb o;
    private eo p;
    private fj q;
    private boolean r;

    public ej(boolean bl2) {
        this.j = bl2;
    }

    @Override
    public void channelActive(ChannelHandlerContext channelHandlerContext) {
        super.channelActive(channelHandlerContext);
        this.m = channelHandlerContext.channel();
        this.n = this.m.remoteAddress();
        this.a(eo.a);
    }

    public void a(eo eo2) {
        this.p = this.m.attr(d).getAndSet(eo2);
        this.m.attr(e).set(eo2.a(this.j));
        this.m.attr(f).set(eo2.b(this.j));
        this.m.config().setAutoRead(true);
        i.debug("Enabled auto read");
    }

    @Override
    public void channelInactive(ChannelHandlerContext channelHandlerContext) {
        this.a(new fr("disconnect.endOfStream", new Object[0]));
    }

    @Override
    public void exceptionCaught(ChannelHandlerContext channelHandlerContext, Throwable throwable) {
        fr fr2 = throwable instanceof TimeoutException ? new fr("disconnect.timeout", new Object[0]) : new fr("disconnect.genericReason", "Internal Exception: " + throwable);
        this.a(fr2);
    }

    protected void a(ChannelHandlerContext channelHandlerContext, ft ft2) {
        if (this.m.isOpen()) {
            if (ft2.a()) {
                ft2.a(this.o);
            } else {
                this.k.add(ft2);
            }
        }
    }

    public void a(fb fb2) {
        Validate.notNull(fb2, "packetListener", new Object[0]);
        i.debug("Set listener of {} to {}", this, fb2);
        this.o = fb2;
    }

    public void a(ft ft2, GenericFutureListener ... genericFutureListenerArray) {
        if (this.m != null && this.m.isOpen()) {
            this.i();
            this.b(ft2, genericFutureListenerArray);
        } else {
            this.l.add(new en(ft2, genericFutureListenerArray));
        }
    }

    private void b(ft ft2, GenericFutureListener[] genericFutureListenerArray) {
        eo eo2 = eo.a(ft2);
        eo eo3 = (eo)((Object)this.m.attr(d).get());
        if (eo3 != eo2) {
            i.debug("Disabled auto read");
            this.m.config().setAutoRead(false);
        }
        if (this.m.eventLoop().inEventLoop()) {
            if (eo2 != eo3) {
                this.a(eo2);
            }
            this.m.writeAndFlush(ft2).addListeners(genericFutureListenerArray).addListener(ChannelFutureListener.FIRE_EXCEPTION_ON_FAILURE);
        } else {
            this.m.eventLoop().execute(new ek(this, eo2, eo3, ft2, genericFutureListenerArray));
        }
    }

    private void i() {
        if (this.m == null || !this.m.isOpen()) {
            return;
        }
        while (!this.l.isEmpty()) {
            en en2 = (en)this.l.poll();
            this.b(en.a(en2), en.b(en2));
        }
    }

    public void a() {
        this.i();
        eo eo2 = (eo)((Object)this.m.attr(d).get());
        if (this.p != eo2) {
            if (this.p != null) {
                this.o.a(this.p, eo2);
            }
            this.p = eo2;
        }
        if (this.o != null) {
            for (int i2 = 1000; !this.k.isEmpty() && i2 >= 0; --i2) {
                ft ft2 = (ft)this.k.poll();
                ft2.a(this.o);
            }
            this.o.a();
        }
        this.m.flush();
    }

    public SocketAddress b() {
        return this.n;
    }

    public void a(fj fj2) {
        if (this.m.isOpen()) {
            this.m.close();
            this.q = fj2;
        }
    }

    public boolean c() {
        return this.m instanceof LocalChannel || this.m instanceof LocalServerChannel;
    }

    public void a(SecretKey secretKey) {
        this.m.pipeline().addBefore("splitter", "decrypt", new eh(pt.a(2, secretKey)));
        this.m.pipeline().addBefore("prepender", "encrypt", new ei(pt.a(1, secretKey)));
        this.r = true;
    }

    public boolean d() {
        return this.m != null && this.m.isOpen();
    }

    public fb e() {
        return this.o;
    }

    public fj f() {
        return this.q;
    }

    public void g() {
        this.m.config().setAutoRead(false);
    }

    protected /* synthetic */ void channelRead0(ChannelHandlerContext channelHandlerContext, Object object) {
        this.a(channelHandlerContext, (ft)object);
    }

    static /* synthetic */ Channel a(ej ej2) {
        return ej2.m;
    }
}

