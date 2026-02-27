/*
 * Decompiled with CFR 0.152.
 */
public class cn
extends cw {
    private final String d;
    private Object e;

    public cn(String string) {
        this.d = string;
    }

    @Override
    public void a(int n2, String string, Object object) {
        if (this.d.equals(string)) {
            this.e = object;
        }
        super.a(n2, string, object);
    }

    @Override
    public Object a(String string) {
        Object object = super.a(string);
        return object == null ? this.e : object;
    }

    @Override
    public Object a(int n2) {
        Object object = super.a(n2);
        return object == null ? this.e : object;
    }
}

