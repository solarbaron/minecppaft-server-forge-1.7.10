/*
 * Decompiled with CFR 0.152.
 */
import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.InputStreamReader;
import java.io.UnsupportedEncodingException;
import java.net.HttpURLConnection;
import java.net.Proxy;
import java.net.URL;
import java.net.URLEncoder;
import java.util.Map;
import java.util.concurrent.atomic.AtomicInteger;
import net.minecraft.server.MinecraftServer;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class pv {
    private static final AtomicInteger a = new AtomicInteger(0);
    private static final Logger b = LogManager.getLogger();

    public static String a(Map map) {
        StringBuilder stringBuilder = new StringBuilder();
        for (Map.Entry entry : map.entrySet()) {
            if (stringBuilder.length() > 0) {
                stringBuilder.append('&');
            }
            try {
                stringBuilder.append(URLEncoder.encode((String)entry.getKey(), "UTF-8"));
            }
            catch (UnsupportedEncodingException unsupportedEncodingException) {
                unsupportedEncodingException.printStackTrace();
            }
            if (entry.getValue() == null) continue;
            stringBuilder.append('=');
            try {
                stringBuilder.append(URLEncoder.encode(entry.getValue().toString(), "UTF-8"));
            }
            catch (UnsupportedEncodingException unsupportedEncodingException) {
                unsupportedEncodingException.printStackTrace();
            }
        }
        return stringBuilder.toString();
    }

    public static String a(URL uRL, Map map, boolean bl2) {
        return pv.a(uRL, pv.a(map), bl2);
    }

    private static String a(URL uRL, String string, boolean bl2) {
        try {
            String string2;
            Proxy proxy;
            Proxy proxy2 = proxy = MinecraftServer.I() == null ? null : MinecraftServer.I().aq();
            if (proxy == null) {
                proxy = Proxy.NO_PROXY;
            }
            HttpURLConnection httpURLConnection = (HttpURLConnection)uRL.openConnection(proxy);
            httpURLConnection.setRequestMethod("POST");
            httpURLConnection.setRequestProperty("Content-Type", "application/x-www-form-urlencoded");
            httpURLConnection.setRequestProperty("Content-Length", "" + string.getBytes().length);
            httpURLConnection.setRequestProperty("Content-Language", "en-US");
            httpURLConnection.setUseCaches(false);
            httpURLConnection.setDoInput(true);
            httpURLConnection.setDoOutput(true);
            DataOutputStream dataOutputStream = new DataOutputStream(httpURLConnection.getOutputStream());
            dataOutputStream.writeBytes(string);
            dataOutputStream.flush();
            dataOutputStream.close();
            BufferedReader bufferedReader = new BufferedReader(new InputStreamReader(httpURLConnection.getInputStream()));
            StringBuffer stringBuffer = new StringBuffer();
            while ((string2 = bufferedReader.readLine()) != null) {
                stringBuffer.append(string2);
                stringBuffer.append('\r');
            }
            bufferedReader.close();
            return stringBuffer.toString();
        }
        catch (Exception exception) {
            if (!bl2) {
                b.error("Could not post to " + uRL, (Throwable)exception);
            }
            return "";
        }
    }
}

