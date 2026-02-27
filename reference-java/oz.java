/*
 * Decompiled with CFR 0.152.
 */
import java.io.BufferedInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.Socket;
import java.net.SocketTimeoutException;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class oz
extends ow {
    private static final Logger h = LogManager.getLogger();
    private boolean i;
    private Socket j;
    private byte[] k = new byte[1460];
    private String l;

    oz(lo lo2, Socket socket) {
        super(lo2, "RCON Client");
        this.j = socket;
        try {
            this.j.setSoTimeout(0);
        }
        catch (Exception exception) {
            this.a = false;
        }
        this.l = lo2.a("rcon.password", "");
        this.b("Rcon connection from: " + socket.getInetAddress());
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    @Override
    public void run() {
        try {
            while (this.a) {
                BufferedInputStream bufferedInputStream = new BufferedInputStream(this.j.getInputStream());
                int n2 = bufferedInputStream.read(this.k, 0, 1460);
                if (10 > n2) {
                    return;
                }
                int n3 = 0;
                int n4 = ou.b(this.k, 0, n2);
                if (n4 != n2 - 4) {
                    return;
                }
                int n5 = ou.b(this.k, n3 += 4, n2);
                int n6 = ou.b(this.k, n3 += 4);
                n3 += 4;
                switch (n6) {
                    case 3: {
                        String string = ou.a(this.k, n3, n2);
                        n3 += string.length();
                        if (0 != string.length() && string.equals(this.l)) {
                            this.i = true;
                            this.a(n5, 2, "");
                            break;
                        }
                        this.i = false;
                        this.f();
                        break;
                    }
                    case 2: {
                        if (this.i) {
                            String string = ou.a(this.k, n3, n2);
                            try {
                                this.a(n5, this.b.g(string));
                            }
                            catch (Exception exception) {
                                this.a(n5, "Error executing: " + string + " (" + exception.getMessage() + ")");
                            }
                            break;
                        }
                        this.f();
                        break;
                    }
                    default: {
                        this.a(n5, String.format("Unknown request %s", Integer.toHexString(n6)));
                    }
                }
            }
        }
        catch (SocketTimeoutException socketTimeoutException) {
        }
        catch (IOException iOException) {
        }
        catch (Exception exception) {
            h.error("Exception whilst parsing RCON input", (Throwable)exception);
        }
        finally {
            this.g();
        }
    }

    private void a(int n2, int n3, String string) {
        ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream(1248);
        DataOutputStream dataOutputStream = new DataOutputStream(byteArrayOutputStream);
        byte[] byArray = string.getBytes("UTF-8");
        dataOutputStream.writeInt(Integer.reverseBytes(byArray.length + 10));
        dataOutputStream.writeInt(Integer.reverseBytes(n2));
        dataOutputStream.writeInt(Integer.reverseBytes(n3));
        dataOutputStream.write(byArray);
        dataOutputStream.write(0);
        dataOutputStream.write(0);
        this.j.getOutputStream().write(byteArrayOutputStream.toByteArray());
    }

    private void f() {
        this.a(-1, 2, "");
    }

    private void a(int n2, String string) {
        int n3;
        int n4 = string.length();
        do {
            n3 = 4096 <= n4 ? 4096 : n4;
            this.a(n2, 0, string.substring(0, n3));
        } while (0 != (n4 = (string = string.substring(n3)).length()));
    }

    private void g() {
        if (null == this.j) {
            return;
        }
        try {
            this.j.close();
        }
        catch (IOException iOException) {
            this.c("IO: " + iOException.getMessage());
        }
        this.j = null;
    }
}

