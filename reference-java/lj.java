/*
 * Decompiled with CFR 0.152.
 */
import net.minecraft.server.MinecraftServer;

public class lj
extends Thread {
    final /* synthetic */ MinecraftServer a;

    public lj(MinecraftServer minecraftServer, String string) {
        this.a = minecraftServer;
        super(string);
    }

    @Override
    public void run() {
        this.a.run();
    }
}

