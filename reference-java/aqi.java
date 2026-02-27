/*
 * Decompiled with CFR 0.152.
 */
import java.io.ByteArrayOutputStream;

class aqi
extends ByteArrayOutputStream {
    private int b;
    private int c;
    final /* synthetic */ aqh a;

    public aqi(aqh aqh2, int n2, int n3) {
        this.a = aqh2;
        super(8096);
        this.b = n2;
        this.c = n3;
    }

    @Override
    public void close() {
        this.a.a(this.b, this.c, this.buf, this.count);
    }
}

