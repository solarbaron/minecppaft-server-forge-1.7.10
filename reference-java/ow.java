/*
 * Decompiled with CFR 0.152.
 */
import java.io.Closeable;
import java.io.IOException;
import java.net.DatagramSocket;
import java.net.ServerSocket;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;

public abstract class ow
implements Runnable {
    private static final AtomicInteger h = new AtomicInteger(0);
    protected boolean a;
    protected lo b;
    protected final String c;
    protected Thread d;
    protected int e = 5;
    protected List f = new ArrayList();
    protected List g = new ArrayList();

    protected ow(lo lo2, String string) {
        this.b = lo2;
        this.c = string;
        if (this.b.H()) {
            this.c("Debugging is enabled, performance maybe reduced!");
        }
    }

    public synchronized void a() {
        this.d = new Thread((Runnable)this, this.c + " #" + h.incrementAndGet());
        this.d.start();
        this.a = true;
    }

    public boolean c() {
        return this.a;
    }

    protected void a(String string) {
        this.b.i(string);
    }

    protected void b(String string) {
        this.b.e(string);
    }

    protected void c(String string) {
        this.b.f(string);
    }

    protected void d(String string) {
        this.b.h(string);
    }

    protected int d() {
        return this.b.C();
    }

    protected void a(DatagramSocket datagramSocket) {
        this.a("registerSocket: " + datagramSocket);
        this.f.add(datagramSocket);
    }

    protected boolean a(DatagramSocket datagramSocket, boolean bl2) {
        this.a("closeSocket: " + datagramSocket);
        if (null == datagramSocket) {
            return false;
        }
        boolean bl3 = false;
        if (!datagramSocket.isClosed()) {
            datagramSocket.close();
            bl3 = true;
        }
        if (bl2) {
            this.f.remove(datagramSocket);
        }
        return bl3;
    }

    protected boolean b(ServerSocket serverSocket) {
        return this.a(serverSocket, true);
    }

    protected boolean a(ServerSocket serverSocket, boolean bl2) {
        this.a("closeSocket: " + serverSocket);
        if (null == serverSocket) {
            return false;
        }
        boolean bl3 = false;
        try {
            if (!serverSocket.isClosed()) {
                serverSocket.close();
                bl3 = true;
            }
        }
        catch (IOException iOException) {
            this.c("IO: " + iOException.getMessage());
        }
        if (bl2) {
            this.g.remove(serverSocket);
        }
        return bl3;
    }

    protected void e() {
        this.a(false);
    }

    protected void a(boolean bl2) {
        int n2 = 0;
        for (Closeable closeable : this.f) {
            if (!this.a((DatagramSocket)closeable, false)) continue;
            ++n2;
        }
        this.f.clear();
        for (Closeable closeable : this.g) {
            if (!this.a((ServerSocket)closeable, false)) continue;
            ++n2;
        }
        this.g.clear();
        if (bl2 && 0 < n2) {
            this.c("Force closed " + n2 + " sockets");
        }
    }
}

