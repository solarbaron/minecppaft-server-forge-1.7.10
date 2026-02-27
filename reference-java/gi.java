/*
 * Decompiled with CFR 0.152.
 */
import org.apache.commons.lang3.ArrayUtils;

public class gi
extends ft {
    private String[] a;

    public gi() {
    }

    public gi(String[] stringArray) {
        this.a = stringArray;
    }

    @Override
    public void a(et et2) {
        this.a = new String[et2.a()];
        for (int i2 = 0; i2 < this.a.length; ++i2) {
            this.a[i2] = et2.c(Short.MAX_VALUE);
        }
    }

    @Override
    public void b(et et2) {
        et2.b(this.a.length);
        for (String string : this.a) {
            et2.a(string);
        }
    }

    public void a(fv fv2) {
        fv2.a(this);
    }

    @Override
    public String b() {
        return String.format("candidates='%s'", ArrayUtils.toString(this.a));
    }
}

