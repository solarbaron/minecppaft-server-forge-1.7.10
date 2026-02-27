/*
 * Decompiled with CFR 0.152.
 */
import java.io.File;

public class aym
extends ayq {
    public aym(File file, String string, boolean bl2) {
        super(file, string, bl2);
    }

    @Override
    public aqc a(aqo aqo2) {
        File file = this.b();
        if (aqo2 instanceof aqp) {
            File file2 = new File(file, "DIM-1");
            file2.mkdirs();
            return new aqk(file2);
        }
        if (aqo2 instanceof aqr) {
            File file3 = new File(file, "DIM1");
            file3.mkdirs();
            return new aqk(file3);
        }
        return new aqk(file);
    }

    @Override
    public void a(ays ays2, dh dh2) {
        ays2.e(19133);
        super.a(ays2, dh2);
    }

    @Override
    public void a() {
        try {
            azr.a.a();
        }
        catch (InterruptedException interruptedException) {
            interruptedException.printStackTrace();
        }
        aqj.a();
    }
}

