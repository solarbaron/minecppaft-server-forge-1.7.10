/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.List;
import net.minecraft.server.MinecraftServer;

public class ahy {
    private final aib a;
    private long b;
    private qd c = new qd();
    private List d = new ArrayList();

    public ahy(aib aib2) {
        this.a = aib2;
    }

    public ahz a(int n2, int n3) {
        long l2 = (long)(n2 >>= 4) & 0xFFFFFFFFL | ((long)(n3 >>= 4) & 0xFFFFFFFFL) << 32;
        ahz ahz2 = (ahz)this.c.a(l2);
        if (ahz2 == null) {
            ahz2 = new ahz(this, n2, n3);
            this.c.a(l2, ahz2);
            this.d.add(ahz2);
        }
        ahz2.e = MinecraftServer.ar();
        return ahz2;
    }

    public ahu b(int n2, int n3) {
        return this.a(n2, n3).a(n2, n3);
    }

    public void a() {
        long l2 = MinecraftServer.ar();
        long l3 = l2 - this.b;
        if (l3 > 7500L || l3 < 0L) {
            this.b = l2;
            for (int i2 = 0; i2 < this.d.size(); ++i2) {
                ahz ahz2 = (ahz)this.d.get(i2);
                long l4 = l2 - ahz2.e;
                if (l4 <= 30000L && l4 >= 0L) continue;
                this.d.remove(i2--);
                long l5 = (long)ahz2.c & 0xFFFFFFFFL | ((long)ahz2.d & 0xFFFFFFFFL) << 32;
                this.c.d(l5);
            }
        }
    }

    public ahu[] d(int n2, int n3) {
        return this.a((int)n2, (int)n3).b;
    }

    static /* synthetic */ aib a(ahy ahy2) {
        return ahy2.a;
    }
}

