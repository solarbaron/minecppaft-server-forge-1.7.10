/*
 * Decompiled with CFR 0.152.
 */
import java.io.File;
import java.io.FileInputStream;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class ayr
implements aze {
    private static final Logger b = LogManager.getLogger();
    protected final File a;

    public ayr(File file) {
        if (!file.exists()) {
            file.mkdirs();
        }
        this.a = file;
    }

    @Override
    public void d() {
    }

    public ays c(String string) {
        File file = new File(this.a, string);
        if (!file.exists()) {
            return null;
        }
        File file2 = new File(file, "level.dat");
        if (file2.exists()) {
            try {
                dh dh2 = du.a(new FileInputStream(file2));
                dh dh3 = dh2.m("Data");
                return new ays(dh3);
            }
            catch (Exception exception) {
                b.error("Exception reading " + file2, (Throwable)exception);
            }
        }
        if ((file2 = new File(file, "level.dat_old")).exists()) {
            try {
                dh dh4 = du.a(new FileInputStream(file2));
                dh dh5 = dh4.m("Data");
                return new ays(dh5);
            }
            catch (Exception exception) {
                b.error("Exception reading " + file2, (Throwable)exception);
            }
        }
        return null;
    }

    @Override
    public boolean e(String string) {
        File file = new File(this.a, string);
        if (!file.exists()) {
            return true;
        }
        b.info("Deleting level " + string);
        for (int i2 = 1; i2 <= 5; ++i2) {
            b.info("Attempt " + i2 + "...");
            if (ayr.a(file.listFiles())) break;
            b.warn("Unsuccessful in deleting contents.");
            if (i2 >= 5) continue;
            try {
                Thread.sleep(500L);
                continue;
            }
            catch (InterruptedException interruptedException) {
                // empty catch block
            }
        }
        return file.delete();
    }

    protected static boolean a(File[] fileArray) {
        for (int i2 = 0; i2 < fileArray.length; ++i2) {
            File file = fileArray[i2];
            b.debug("Deleting " + file);
            if (file.isDirectory() && !ayr.a(file.listFiles())) {
                b.warn("Couldn't delete directory " + file);
                return false;
            }
            if (file.delete()) continue;
            b.warn("Couldn't delete file " + file);
            return false;
        }
        return true;
    }

    @Override
    public azc a(String string, boolean bl2) {
        return new ayq(this.a, string, bl2);
    }

    @Override
    public boolean b(String string) {
        return false;
    }

    @Override
    public boolean a(String string, qk qk2) {
        return false;
    }
}

