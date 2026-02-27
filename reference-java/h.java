/*
 * Decompiled with CFR 0.152.
 */
import java.lang.management.ManagementFactory;
import java.lang.management.RuntimeMXBean;
import java.util.List;
import java.util.concurrent.Callable;

class h
implements Callable {
    final /* synthetic */ b a;

    h(b b2) {
        this.a = b2;
    }

    public String a() {
        RuntimeMXBean runtimeMXBean = ManagementFactory.getRuntimeMXBean();
        List<String> list = runtimeMXBean.getInputArguments();
        int n2 = 0;
        StringBuilder stringBuilder = new StringBuilder();
        for (String string : list) {
            if (!string.startsWith("-X")) continue;
            if (n2++ > 0) {
                stringBuilder.append(" ");
            }
            stringBuilder.append(string);
        }
        return String.format("%d total; %s", n2, stringBuilder.toString());
    }

    public /* synthetic */ Object call() {
        return this.a();
    }
}

