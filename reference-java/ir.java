/*
 * Decompiled with CFR 0.152.
 */
public class ir
extends ft {
    private String a;

    public ir() {
    }

    public ir(String string) {
        if (string.length() > 100) {
            string = string.substring(0, 100);
        }
        this.a = string;
    }

    @Override
    public void a(et et2) {
        this.a = et2.c(100);
    }

    @Override
    public void b(et et2) {
        et2.a(this.a);
    }

    public void a(io io2) {
        io2.a(this);
    }

    @Override
    public String b() {
        return String.format("message='%s'", this.a);
    }

    public String c() {
        return this.a;
    }
}

