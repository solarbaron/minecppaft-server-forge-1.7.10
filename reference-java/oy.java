/*
 * Decompiled with CFR 0.152.
 */
import java.net.DatagramPacket;
import java.util.Date;
import java.util.Random;

class oy {
    private long b = new Date().getTime();
    private int c;
    private byte[] d;
    private byte[] e;
    private String f;
    final /* synthetic */ ox a;

    public oy(ox ox2, DatagramPacket datagramPacket) {
        this.a = ox2;
        byte[] byArray = datagramPacket.getData();
        this.d = new byte[4];
        this.d[0] = byArray[3];
        this.d[1] = byArray[4];
        this.d[2] = byArray[5];
        this.d[3] = byArray[6];
        this.f = new String(this.d);
        this.c = new Random().nextInt(0x1000000);
        this.e = String.format("\t%s%d\u0000", this.f, this.c).getBytes();
    }

    public Boolean a(long l2) {
        return this.b < l2;
    }

    public int a() {
        return this.c;
    }

    public byte[] b() {
        return this.e;
    }

    public byte[] c() {
        return this.d;
    }
}

