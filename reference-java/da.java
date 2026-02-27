/*
 * Decompiled with CFR 0.152.
 */
import com.google.common.collect.Maps;
import java.util.Collections;
import java.util.Map;
import java.util.Set;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class da
implements cz {
    private static final Logger a = LogManager.getLogger();
    protected final Map c = this.a();

    protected Map a() {
        return Maps.newHashMap();
    }

    @Override
    public Object a(Object object) {
        return this.c.get(object);
    }

    @Override
    public void a(Object object, Object object2) {
        if (this.c.containsKey(object)) {
            a.debug("Adding duplicate key '" + object + "' to registry");
        }
        this.c.put(object, object2);
    }

    public Set b() {
        return Collections.unmodifiableSet(this.c.keySet());
    }

    public boolean d(Object object) {
        return this.c.containsKey(object);
    }
}

