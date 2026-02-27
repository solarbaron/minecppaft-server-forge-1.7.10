/*
 * Decompiled with CFR 0.152.
 */
import java.io.DataInputStream;
import java.io.DataOutput;
import java.io.DataOutputStream;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class ayn
extends ayr {
    private static final Logger b = LogManager.getLogger();

    public ayn(File file) {
        super(file);
    }

    protected int c() {
        return 19133;
    }

    @Override
    public void d() {
        aqj.a();
    }

    @Override
    public azc a(String string, boolean bl2) {
        return new aym(this.a, string, bl2);
    }

    @Override
    public boolean b(String string) {
        ays ays2 = this.c(string);
        return ays2 != null && ays2.l() != this.c();
    }

    @Override
    public boolean a(String string, qk qk2) {
        qk2.a(0);
        ArrayList arrayList = new ArrayList();
        ArrayList arrayList2 = new ArrayList();
        ArrayList arrayList3 = new ArrayList();
        File file = new File(this.a, string);
        File file2 = new File(file, "DIM-1");
        File file3 = new File(file, "DIM1");
        b.info("Scanning folders...");
        this.a(file, arrayList);
        if (file2.exists()) {
            this.a(file2, arrayList2);
        }
        if (file3.exists()) {
            this.a(file3, arrayList3);
        }
        int n2 = arrayList.size() + arrayList2.size() + arrayList3.size();
        b.info("Total conversion count is " + n2);
        ays ays2 = this.c(string);
        aib aib2 = null;
        aib2 = ays2.u() == ahm.c ? new aie(ahu.p, 0.5f) : new aib(ays2.b(), ays2.u());
        this.a(new File(file, "region"), arrayList, aib2, 0, n2, qk2);
        this.a(new File(file2, "region"), arrayList2, (aib)new aie(ahu.w, 0.0f), arrayList.size(), n2, qk2);
        this.a(new File(file3, "region"), arrayList3, (aib)new aie(ahu.x, 0.0f), arrayList.size() + arrayList2.size(), n2, qk2);
        ays2.e(19133);
        if (ays2.u() == ahm.f) {
            ays2.a(ahm.b);
        }
        this.g(string);
        azc azc2 = this.a(string, false);
        azc2.a(ays2);
        return true;
    }

    private void g(String string) {
        File file = new File(this.a, string);
        if (!file.exists()) {
            b.warn("Unable to create level.dat_mcr backup");
            return;
        }
        File file2 = new File(file, "level.dat");
        if (!file2.exists()) {
            b.warn("Unable to create level.dat_mcr backup");
            return;
        }
        File file3 = new File(file, "level.dat_mcr");
        if (!file2.renameTo(file3)) {
            b.warn("Unable to create level.dat_mcr backup");
        }
    }

    private void a(File file, Iterable iterable, aib aib2, int n2, int n3, qk qk2) {
        for (File file2 : iterable) {
            this.a(file, file2, aib2, n2, n3, qk2);
            int n4 = (int)Math.round(100.0 * (double)(++n2) / (double)n3);
            qk2.a(n4);
        }
    }

    private void a(File file, File file2, aib aib2, int n2, int n3, qk qk2) {
        try {
            String string = file2.getName();
            aqh aqh2 = new aqh(file2);
            aqh aqh3 = new aqh(new File(file, string.substring(0, string.length() - ".mcr".length()) + ".mca"));
            for (int i2 = 0; i2 < 32; ++i2) {
                int n4;
                for (n4 = 0; n4 < 32; ++n4) {
                    if (!aqh2.c(i2, n4) || aqh3.c(i2, n4)) continue;
                    DataInputStream dataInputStream = aqh2.a(i2, n4);
                    if (dataInputStream == null) {
                        b.warn("Failed to fetch input stream");
                        continue;
                    }
                    dh dh2 = du.a(dataInputStream);
                    dataInputStream.close();
                    dh dh3 = dh2.m("Level");
                    aqg aqg2 = aqf.a(dh3);
                    dh dh4 = new dh();
                    dh dh5 = new dh();
                    dh4.a("Level", dh5);
                    aqf.a(aqg2, dh5, aib2);
                    DataOutputStream dataOutputStream = aqh3.b(i2, n4);
                    du.a(dh4, (DataOutput)dataOutputStream);
                    dataOutputStream.close();
                }
                n4 = (int)Math.round(100.0 * (double)(n2 * 1024) / (double)(n3 * 1024));
                int n5 = (int)Math.round(100.0 * (double)((i2 + 1) * 32 + n2 * 1024) / (double)(n3 * 1024));
                if (n5 <= n4) continue;
                qk2.a(n5);
            }
            aqh2.c();
            aqh3.c();
        }
        catch (IOException iOException) {
            iOException.printStackTrace();
        }
    }

    private void a(File file, Collection collection) {
        File file2 = new File(file, "region");
        File[] fileArray = file2.listFiles(new ayo(this));
        if (fileArray != null) {
            Collections.addAll(collection, fileArray);
        }
    }
}

