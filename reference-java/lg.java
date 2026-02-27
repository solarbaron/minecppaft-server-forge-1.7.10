/*
 * Decompiled with CFR 0.152.
 */
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.util.Properties;
import org.apache.commons.io.IOUtils;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class lg {
    private static final Logger a = LogManager.getLogger();
    private final File b;
    private final boolean c;

    public lg(File file) {
        this.b = file;
        this.c = this.a(file);
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    private boolean a(File file) {
        FileInputStream fileInputStream = null;
        boolean bl2 = false;
        try {
            Properties properties = new Properties();
            fileInputStream = new FileInputStream(file);
            properties.load(fileInputStream);
            bl2 = Boolean.parseBoolean(properties.getProperty("eula", "false"));
        }
        catch (Exception exception) {
            try {
                a.warn("Failed to load " + file);
                this.b();
            }
            catch (Throwable throwable) {
                IOUtils.closeQuietly(fileInputStream);
                throw throwable;
            }
            IOUtils.closeQuietly(fileInputStream);
        }
        IOUtils.closeQuietly(fileInputStream);
        return bl2;
    }

    public boolean a() {
        return this.c;
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    public void b() {
        FileOutputStream fileOutputStream = null;
        try {
            Properties properties = new Properties();
            fileOutputStream = new FileOutputStream(this.b);
            properties.setProperty("eula", "false");
            properties.store(fileOutputStream, "By changing the setting below to TRUE you are indicating your agreement to our EULA (https://account.mojang.com/documents/minecraft_eula).");
        }
        catch (Exception exception) {
            try {
                a.warn("Failed to save " + this.b, (Throwable)exception);
            }
            catch (Throwable throwable) {
                IOUtils.closeQuietly(fileOutputStream);
                throw throwable;
            }
            IOUtils.closeQuietly(fileOutputStream);
        }
        IOUtils.closeQuietly(fileOutputStream);
    }
}

