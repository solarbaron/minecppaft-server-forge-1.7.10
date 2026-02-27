/*
 * Decompiled with CFR 0.152.
 */
import org.apache.commons.lang3.StringUtils;

public class iq
extends ft {
    private String a;

    public iq() {
    }

    public iq(String string) {
        this.a = string;
    }

    @Override
    public void a(et et2) {
        this.a = et2.c(Short.MAX_VALUE);
    }

    @Override
    public void b(et et2) {
        et2.a(StringUtils.substring(this.a, 0, Short.MAX_VALUE));
    }

    public void a(io io2) {
        io2.a(this);
    }

    public String c() {
        return this.a;
    }

    @Override
    public String b() {
        return String.format("message='%s'", this.a);
    }
}

