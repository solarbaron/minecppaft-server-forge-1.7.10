/*
 * Decompiled with CFR 0.152.
 */
import java.io.IOException;
import java.net.InetAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketTimeoutException;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

public class pa
extends ow {
    private int h;
    private int i;
    private String j;
    private ServerSocket k;
    private String l;
    private Map m;

    public pa(lo lo2) {
        super(lo2, "RCON Listener");
        this.h = lo2.a("rcon.port", 0);
        this.l = lo2.a("rcon.password", "");
        this.j = lo2.y();
        this.i = lo2.z();
        if (0 == this.h) {
            this.h = this.i + 10;
            this.b("Setting default rcon port to " + this.h);
            lo2.a("rcon.port", (Object)this.h);
            if (0 == this.l.length()) {
                lo2.a("rcon.password", (Object)"");
            }
            lo2.a();
        }
        if (0 == this.j.length()) {
            this.j = "0.0.0.0";
        }
        this.f();
        this.k = null;
    }

    private void f() {
        this.m = new HashMap();
    }

    private void g() {
        Iterator iterator = this.m.entrySet().iterator();
        while (iterator.hasNext()) {
            Map.Entry entry = iterator.next();
            if (((oz)entry.getValue()).c()) continue;
            iterator.remove();
        }
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    @Override
    public void run() {
        this.b("RCON running on " + this.j + ":" + this.h);
        try {
            while (this.a) {
                try {
                    Socket socket = this.k.accept();
                    socket.setSoTimeout(500);
                    oz oz2 = new oz(this.b, socket);
                    oz2.a();
                    this.m.put(socket.getRemoteSocketAddress(), oz2);
                    this.g();
                }
                catch (SocketTimeoutException socketTimeoutException) {
                    this.g();
                }
                catch (IOException iOException) {
                    if (!this.a) continue;
                    this.b("IO: " + iOException.getMessage());
                }
            }
        }
        finally {
            this.b(this.k);
        }
    }

    @Override
    public void a() {
        if (0 == this.l.length()) {
            this.c("No rcon password set in '" + this.b.b() + "', rcon disabled!");
            return;
        }
        if (0 >= this.h || 65535 < this.h) {
            this.c("Invalid rcon port " + this.h + " found in '" + this.b.b() + "', rcon disabled!");
            return;
        }
        if (this.a) {
            return;
        }
        try {
            this.k = new ServerSocket(this.h, 0, InetAddress.getByName(this.j));
            this.k.setSoTimeout(500);
            super.a();
        }
        catch (IOException iOException) {
            this.c("Unable to initialise rcon on " + this.j + ":" + this.h + " : " + iOException.getMessage());
        }
    }
}

