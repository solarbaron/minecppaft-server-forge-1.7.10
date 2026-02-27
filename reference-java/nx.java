/*
 * Decompiled with CFR 0.152.
 */
import com.google.gson.JsonObject;
import java.io.File;
import java.net.SocketAddress;

public class nx
extends om {
    public nx(File file) {
        super(file);
    }

    @Override
    protected ol a(JsonObject jsonObject) {
        return new ny(jsonObject);
    }

    public boolean a(SocketAddress socketAddress) {
        String string = this.c(socketAddress);
        return this.d(string);
    }

    public ny b(SocketAddress socketAddress) {
        String string = this.c(socketAddress);
        return (ny)this.b(string);
    }

    private String c(SocketAddress socketAddress) {
        String string = socketAddress.toString();
        if (string.contains("/")) {
            string = string.substring(string.indexOf(47) + 1);
        }
        if (string.contains(":")) {
            string = string.substring(0, string.indexOf(58));
        }
        return string;
    }
}

