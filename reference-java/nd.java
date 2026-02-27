/*
 * Decompiled with CFR 0.152.
 */
import io.netty.channel.Channel;
import io.netty.channel.ChannelException;
import io.netty.channel.ChannelHandler;
import io.netty.channel.ChannelInitializer;
import io.netty.channel.ChannelOption;
import io.netty.handler.timeout.ReadTimeoutHandler;

class nd
extends ChannelInitializer {
    final /* synthetic */ nc a;

    nd(nc nc2) {
        this.a = nc2;
    }

    protected void initChannel(Channel channel) {
        try {
            channel.config().setOption(ChannelOption.IP_TOS, 24);
        }
        catch (ChannelException channelException) {
            // empty catch block
        }
        try {
            channel.config().setOption(ChannelOption.TCP_NODELAY, false);
        }
        catch (ChannelException channelException) {
            // empty catch block
        }
        channel.pipeline().addLast("timeout", (ChannelHandler)new ReadTimeoutHandler(30)).addLast("legacy_query", (ChannelHandler)new mz(this.a)).addLast("splitter", (ChannelHandler)new fc()).addLast("decoder", (ChannelHandler)new ez(ej.h)).addLast("prepender", (ChannelHandler)new fd()).addLast("encoder", (ChannelHandler)new fa(ej.h));
        ej ej2 = new ej(false);
        nc.a(this.a).add(ej2);
        channel.pipeline().addLast("packet_handler", (ChannelHandler)ej2);
        ej2.a(new nl(nc.b(this.a), ej2));
    }
}

