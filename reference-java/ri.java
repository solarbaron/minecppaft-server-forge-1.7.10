/*
 * Decompiled with CFR 0.152.
 */
import com.google.common.collect.Maps;
import java.lang.management.ManagementFactory;
import java.lang.management.RuntimeMXBean;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.List;
import java.util.Map;
import java.util.Timer;
import java.util.TimerTask;
import java.util.UUID;

public class ri {
    private final Map a = Maps.newHashMap();
    private final Map b = Maps.newHashMap();
    private final String c = UUID.randomUUID().toString();
    private final URL d;
    private final rk e;
    private final Timer f = new Timer("Snooper Timer", true);
    private final Object g = new Object();
    private final long h;
    private boolean i;
    private int j;

    public ri(String string, rk rk2, long l2) {
        try {
            this.d = new URL("http://snoop.minecraft.net/" + string + "?version=" + 2);
        }
        catch (MalformedURLException malformedURLException) {
            throw new IllegalArgumentException();
        }
        this.e = rk2;
        this.h = l2;
    }

    public void a() {
        if (this.i) {
            return;
        }
        this.i = true;
        this.h();
        this.f.schedule((TimerTask)new rj(this), 0L, 900000L);
    }

    private void h() {
        this.i();
        this.a("snooper_token", this.c);
        this.b("snooper_token", this.c);
        this.b("os_name", System.getProperty("os.name"));
        this.b("os_version", System.getProperty("os.version"));
        this.b("os_architecture", System.getProperty("os.arch"));
        this.b("java_version", System.getProperty("java.version"));
        this.b("version", "1.7.10");
        this.e.b(this);
    }

    private void i() {
        RuntimeMXBean runtimeMXBean = ManagementFactory.getRuntimeMXBean();
        List<String> list = runtimeMXBean.getInputArguments();
        int n2 = 0;
        for (String string : list) {
            if (!string.startsWith("-X")) continue;
            this.a("jvm_arg[" + n2++ + "]", string);
        }
        this.a("jvm_args", n2);
    }

    public void b() {
        this.b("memory_total", Runtime.getRuntime().totalMemory());
        this.b("memory_max", Runtime.getRuntime().maxMemory());
        this.b("memory_free", Runtime.getRuntime().freeMemory());
        this.b("cpu_cores", Runtime.getRuntime().availableProcessors());
        this.e.a(this);
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    public void a(String string, Object object) {
        Object object2 = this.g;
        synchronized (object2) {
            this.b.put(string, object);
        }
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    public void b(String string, Object object) {
        Object object2 = this.g;
        synchronized (object2) {
            this.a.put(string, object);
        }
    }

    public boolean d() {
        return this.i;
    }

    public void e() {
        this.f.cancel();
    }

    public long g() {
        return this.h;
    }

    static /* synthetic */ rk a(ri ri2) {
        return ri2.e;
    }

    static /* synthetic */ Object b(ri ri2) {
        return ri2.g;
    }

    static /* synthetic */ Map c(ri ri2) {
        return ri2.b;
    }

    static /* synthetic */ int d(ri ri2) {
        return ri2.j;
    }

    static /* synthetic */ Map e(ri ri2) {
        return ri2.a;
    }

    static /* synthetic */ int f(ri ri2) {
        return ri2.j++;
    }

    static /* synthetic */ String g(ri ri2) {
        return ri2.c;
    }

    static /* synthetic */ URL h(ri ri2) {
        return ri2.d;
    }
}

