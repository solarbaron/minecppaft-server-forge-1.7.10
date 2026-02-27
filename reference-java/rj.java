/*
 * Decompiled with CFR 0.152.
 */
import java.util.HashMap;
import java.util.TimerTask;

class rj
extends TimerTask {
    final /* synthetic */ ri a;

    rj(ri ri2) {
        this.a = ri2;
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    @Override
    public void run() {
        HashMap<String, Object> hashMap;
        if (!ri.a(this.a).W()) {
            return;
        }
        Object object = ri.b(this.a);
        synchronized (object) {
            hashMap = new HashMap<String, Object>(ri.c(this.a));
            if (ri.d(this.a) == 0) {
                hashMap.putAll(ri.e(this.a));
            }
            hashMap.put("snooper_count", ri.f(this.a));
            hashMap.put("snooper_token", ri.g(this.a));
        }
        pv.a(ri.h(this.a), hashMap, true);
    }
}

