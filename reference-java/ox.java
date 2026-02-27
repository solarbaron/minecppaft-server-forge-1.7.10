/*
 * Decompiled with CFR 0.152.
 */
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.PortUnreachableException;
import java.net.SocketAddress;
import java.net.SocketException;
import java.net.SocketTimeoutException;
import java.net.UnknownHostException;
import java.util.Date;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import net.minecraft.server.MinecraftServer;

public class ox
extends ow {
    private long h;
    private int i;
    private int j;
    private int k;
    private String l;
    private String m;
    private DatagramSocket n;
    private byte[] o = new byte[1460];
    private DatagramPacket p;
    private Map q;
    private String r;
    private String s;
    private Map t;
    private long u;
    private ot v;
    private long w;

    public ox(lo lo2) {
        super(lo2, "Query Listener");
        this.i = lo2.a("query.port", 0);
        this.s = lo2.y();
        this.j = lo2.z();
        this.l = lo2.A();
        this.k = lo2.D();
        this.m = lo2.O();
        this.w = 0L;
        this.r = "0.0.0.0";
        if (0 == this.s.length() || this.r.equals(this.s)) {
            this.s = "0.0.0.0";
            try {
                InetAddress inetAddress = InetAddress.getLocalHost();
                this.r = inetAddress.getHostAddress();
            }
            catch (UnknownHostException unknownHostException) {
                this.c("Unable to determine local host IP, please set server-ip in '" + lo2.b() + "' : " + unknownHostException.getMessage());
            }
        } else {
            this.r = this.s;
        }
        if (0 == this.i) {
            this.i = this.j;
            this.b("Setting default query port to " + this.i);
            lo2.a("query.port", (Object)this.i);
            lo2.a("debug", false);
            lo2.a();
        }
        this.q = new HashMap();
        this.v = new ot(1460);
        this.t = new HashMap();
        this.u = new Date().getTime();
    }

    private void a(byte[] byArray, DatagramPacket datagramPacket) {
        this.n.send(new DatagramPacket(byArray, byArray.length, datagramPacket.getSocketAddress()));
    }

    private boolean a(DatagramPacket datagramPacket) {
        byte[] byArray = datagramPacket.getData();
        int n2 = datagramPacket.getLength();
        SocketAddress socketAddress = datagramPacket.getSocketAddress();
        this.a("Packet len " + n2 + " [" + socketAddress + "]");
        if (3 > n2 || -2 != byArray[0] || -3 != byArray[1]) {
            this.a("Invalid packet [" + socketAddress + "]");
            return false;
        }
        this.a("Packet '" + ou.a(byArray[2]) + "' [" + socketAddress + "]");
        switch (byArray[2]) {
            case 9: {
                this.d(datagramPacket);
                this.a("Challenge [" + socketAddress + "]");
                return true;
            }
            case 0: {
                if (!this.c(datagramPacket).booleanValue()) {
                    this.a("Invalid challenge [" + socketAddress + "]");
                    return false;
                }
                if (15 == n2) {
                    this.a(this.b(datagramPacket), datagramPacket);
                    this.a("Rules [" + socketAddress + "]");
                    break;
                }
                ot ot2 = new ot(1460);
                ot2.a(0);
                ot2.a(this.a(datagramPacket.getSocketAddress()));
                ot2.a(this.l);
                ot2.a("SMP");
                ot2.a(this.m);
                ot2.a(Integer.toString(this.d()));
                ot2.a(Integer.toString(this.k));
                ot2.a((short)this.j);
                ot2.a(this.r);
                this.a(ot2.a(), datagramPacket);
                this.a("Status [" + socketAddress + "]");
            }
        }
        return true;
    }

    private byte[] b(DatagramPacket datagramPacket) {
        String[] stringArray;
        long l2 = MinecraftServer.ar();
        if (l2 < this.w + 5000L) {
            byte[] byArray = this.v.a();
            byte[] byArray2 = this.a(datagramPacket.getSocketAddress());
            byArray[1] = byArray2[0];
            byArray[2] = byArray2[1];
            byArray[3] = byArray2[2];
            byArray[4] = byArray2[3];
            return byArray;
        }
        this.w = l2;
        this.v.b();
        this.v.a(0);
        this.v.a(this.a(datagramPacket.getSocketAddress()));
        this.v.a("splitnum");
        this.v.a(128);
        this.v.a(0);
        this.v.a("hostname");
        this.v.a(this.l);
        this.v.a("gametype");
        this.v.a("SMP");
        this.v.a("game_id");
        this.v.a("MINECRAFT");
        this.v.a("version");
        this.v.a(this.b.B());
        this.v.a("plugins");
        this.v.a(this.b.G());
        this.v.a("map");
        this.v.a(this.m);
        this.v.a("numplayers");
        this.v.a("" + this.d());
        this.v.a("maxplayers");
        this.v.a("" + this.k);
        this.v.a("hostport");
        this.v.a("" + this.j);
        this.v.a("hostip");
        this.v.a(this.r);
        this.v.a(0);
        this.v.a(1);
        this.v.a("player_");
        this.v.a(0);
        for (String string : stringArray = this.b.E()) {
            this.v.a(string);
        }
        this.v.a(0);
        return this.v.a();
    }

    private byte[] a(SocketAddress socketAddress) {
        return ((oy)this.t.get(socketAddress)).c();
    }

    private Boolean c(DatagramPacket datagramPacket) {
        SocketAddress socketAddress = datagramPacket.getSocketAddress();
        if (!this.t.containsKey(socketAddress)) {
            return false;
        }
        byte[] byArray = datagramPacket.getData();
        if (((oy)this.t.get(socketAddress)).a() != ou.c(byArray, 7, datagramPacket.getLength())) {
            return false;
        }
        return true;
    }

    private void d(DatagramPacket datagramPacket) {
        oy oy2 = new oy(this, datagramPacket);
        this.t.put(datagramPacket.getSocketAddress(), oy2);
        this.a(oy2.b(), datagramPacket);
    }

    private void f() {
        if (!this.a) {
            return;
        }
        long l2 = MinecraftServer.ar();
        if (l2 < this.h + 30000L) {
            return;
        }
        this.h = l2;
        Iterator iterator = this.t.entrySet().iterator();
        while (iterator.hasNext()) {
            Map.Entry entry = iterator.next();
            if (!((oy)entry.getValue()).a(l2).booleanValue()) continue;
            iterator.remove();
        }
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    @Override
    public void run() {
        this.b("Query running on " + this.s + ":" + this.i);
        this.h = MinecraftServer.ar();
        this.p = new DatagramPacket(this.o, this.o.length);
        try {
            while (this.a) {
                try {
                    this.n.receive(this.p);
                    this.f();
                    this.a(this.p);
                }
                catch (SocketTimeoutException socketTimeoutException) {
                    this.f();
                }
                catch (PortUnreachableException portUnreachableException) {
                }
                catch (IOException iOException) {
                    this.a(iOException);
                }
            }
        }
        finally {
            this.e();
        }
    }

    @Override
    public void a() {
        if (this.a) {
            return;
        }
        if (0 >= this.i || 65535 < this.i) {
            this.c("Invalid query port " + this.i + " found in '" + this.b.b() + "' (queries disabled)");
            return;
        }
        if (this.g()) {
            super.a();
        }
    }

    private void a(Exception exception) {
        if (!this.a) {
            return;
        }
        this.c("Unexpected exception, buggy JRE? (" + exception.toString() + ")");
        if (!this.g()) {
            this.d("Failed to recover from buggy JRE, shutting down!");
            this.a = false;
        }
    }

    private boolean g() {
        try {
            this.n = new DatagramSocket(this.i, InetAddress.getByName(this.s));
            this.a(this.n);
            this.n.setSoTimeout(500);
            return true;
        }
        catch (SocketException socketException) {
            this.c("Unable to initialise query system on " + this.s + ":" + this.i + " (Socket): " + socketException.getMessage());
        }
        catch (UnknownHostException unknownHostException) {
            this.c("Unable to initialise query system on " + this.s + ":" + this.i + " (Unknown Host): " + unknownHostException.getMessage());
        }
        catch (Exception exception) {
            this.c("Unable to initialise query system on " + this.s + ":" + this.i + " (E): " + exception.getMessage());
        }
        return false;
    }
}

