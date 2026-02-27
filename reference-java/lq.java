/*
 * Decompiled with CFR 0.152.
 */
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Properties;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class lq {
    private static final Logger a = LogManager.getLogger();
    private final Properties b = new Properties();
    private final File c;

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    public lq(File file) {
        this.c = file;
        if (file.exists()) {
            FileInputStream fileInputStream = null;
            try {
                fileInputStream = new FileInputStream(file);
                this.b.load(fileInputStream);
            }
            catch (Exception exception) {
                a.warn("Failed to load " + file, (Throwable)exception);
                this.a();
            }
            finally {
                if (fileInputStream != null) {
                    try {
                        fileInputStream.close();
                    }
                    catch (IOException iOException) {}
                }
            }
        } else {
            a.warn(file + " does not exist");
            this.a();
        }
    }

    public void a() {
        a.info("Generating new properties file");
        this.b();
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    public void b() {
        FileOutputStream fileOutputStream = null;
        try {
            fileOutputStream = new FileOutputStream(this.c);
            this.b.store(fileOutputStream, "Minecraft server properties");
        }
        catch (Exception exception) {
            a.warn("Failed to save " + this.c, (Throwable)exception);
            this.a();
        }
        finally {
            if (fileOutputStream != null) {
                try {
                    fileOutputStream.close();
                }
                catch (IOException iOException) {}
            }
        }
    }

    public File c() {
        return this.c;
    }

    public String a(String string, String string2) {
        if (!this.b.containsKey(string)) {
            this.b.setProperty(string, string2);
            this.b();
            this.b();
        }
        return this.b.getProperty(string, string2);
    }

    public int a(String string, int n2) {
        try {
            return Integer.parseInt(this.a(string, "" + n2));
        }
        catch (Exception exception) {
            this.b.setProperty(string, "" + n2);
            this.b();
            return n2;
        }
    }

    public boolean a(String string, boolean bl2) {
        try {
            return Boolean.parseBoolean(this.a(string, "" + bl2));
        }
        catch (Exception exception) {
            this.b.setProperty(string, "" + bl2);
            this.b();
            return bl2;
        }
    }

    public void a(String string, Object object) {
        this.b.setProperty(string, "" + object);
    }
}

