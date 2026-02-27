/*
 * Decompiled with CFR 0.152.
 */
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import net.minecraft.server.MinecraftServer;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class ayq
implements azc,
azp {
    private static final Logger a = LogManager.getLogger();
    private final File b;
    private final File c;
    private final File d;
    private final long e = MinecraftServer.ar();
    private final String f;

    public ayq(File file, String string, boolean bl2) {
        this.b = new File(file, string);
        this.b.mkdirs();
        this.c = new File(this.b, "playerdata");
        this.d = new File(this.b, "data");
        this.d.mkdirs();
        this.f = string;
        if (bl2) {
            this.c.mkdirs();
        }
        this.h();
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    private void h() {
        try {
            File file = new File(this.b, "session.lock");
            DataOutputStream dataOutputStream = new DataOutputStream(new FileOutputStream(file));
            try {
                dataOutputStream.writeLong(this.e);
            }
            finally {
                dataOutputStream.close();
            }
        }
        catch (IOException iOException) {
            iOException.printStackTrace();
            throw new RuntimeException("Failed to check session lock, aborting");
        }
    }

    @Override
    public File b() {
        return this.b;
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    @Override
    public void c() {
        try {
            File file = new File(this.b, "session.lock");
            DataInputStream dataInputStream = new DataInputStream(new FileInputStream(file));
            try {
                if (dataInputStream.readLong() != this.e) {
                    throw new ahg("The save is being accessed from another location, aborting");
                }
            }
            finally {
                dataInputStream.close();
            }
        }
        catch (IOException iOException) {
            throw new ahg("Failed to check session lock, aborting");
        }
    }

    @Override
    public aqc a(aqo aqo2) {
        throw new RuntimeException("Old Chunk Storage is no longer supported.");
    }

    @Override
    public ays d() {
        File file = new File(this.b, "level.dat");
        if (file.exists()) {
            try {
                dh dh2 = du.a(new FileInputStream(file));
                dh dh3 = dh2.m("Data");
                return new ays(dh3);
            }
            catch (Exception exception) {
                exception.printStackTrace();
            }
        }
        if ((file = new File(this.b, "level.dat_old")).exists()) {
            try {
                dh dh4 = du.a(new FileInputStream(file));
                dh dh5 = dh4.m("Data");
                return new ays(dh5);
            }
            catch (Exception exception) {
                exception.printStackTrace();
            }
        }
        return null;
    }

    @Override
    public void a(ays ays2, dh dh2) {
        dh dh3 = ays2.a(dh2);
        dh dh4 = new dh();
        dh4.a("Data", dh3);
        try {
            File file = new File(this.b, "level.dat_new");
            File file2 = new File(this.b, "level.dat_old");
            File file3 = new File(this.b, "level.dat");
            du.a(dh4, new FileOutputStream(file));
            if (file2.exists()) {
                file2.delete();
            }
            file3.renameTo(file2);
            if (file3.exists()) {
                file3.delete();
            }
            file.renameTo(file3);
            if (file.exists()) {
                file.delete();
            }
        }
        catch (Exception exception) {
            exception.printStackTrace();
        }
    }

    @Override
    public void a(ays ays2) {
        dh dh2 = ays2.a();
        dh dh3 = new dh();
        dh3.a("Data", dh2);
        try {
            File file = new File(this.b, "level.dat_new");
            File file2 = new File(this.b, "level.dat_old");
            File file3 = new File(this.b, "level.dat");
            du.a(dh3, new FileOutputStream(file));
            if (file2.exists()) {
                file2.delete();
            }
            file3.renameTo(file2);
            if (file3.exists()) {
                file3.delete();
            }
            file.renameTo(file3);
            if (file.exists()) {
                file.delete();
            }
        }
        catch (Exception exception) {
            exception.printStackTrace();
        }
    }

    @Override
    public void a(yz yz2) {
        try {
            dh dh2 = new dh();
            yz2.e(dh2);
            File file = new File(this.c, yz2.aB().toString() + ".dat.tmp");
            File file2 = new File(this.c, yz2.aB().toString() + ".dat");
            du.a(dh2, new FileOutputStream(file));
            if (file2.exists()) {
                file2.delete();
            }
            file.renameTo(file2);
        }
        catch (Exception exception) {
            a.warn("Failed to save player data for " + yz2.b_());
        }
    }

    @Override
    public dh b(yz yz2) {
        dh dh2 = null;
        try {
            File file = new File(this.c, yz2.aB().toString() + ".dat");
            if (file.exists() && file.isFile()) {
                dh2 = du.a(new FileInputStream(file));
            }
        }
        catch (Exception exception) {
            a.warn("Failed to load player data for " + yz2.b_());
        }
        if (dh2 != null) {
            yz2.f(dh2);
        }
        return dh2;
    }

    @Override
    public azp e() {
        return this;
    }

    @Override
    public String[] f() {
        String[] stringArray = this.c.list();
        for (int i2 = 0; i2 < stringArray.length; ++i2) {
            if (!stringArray[i2].endsWith(".dat")) continue;
            stringArray[i2] = stringArray[i2].substring(0, stringArray[i2].length() - 4);
        }
        return stringArray;
    }

    @Override
    public void a() {
    }

    @Override
    public File a(String string) {
        return new File(this.d, string + ".dat");
    }

    @Override
    public String g() {
        return this.f;
    }
}

