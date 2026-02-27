/*
 * Decompiled with CFR 0.152.
 */
import java.util.Comparator;
import java.util.List;

public class azz {
    public static final Comparator a = new baa();
    private final bac b;
    private final azx c;
    private final String d;
    private int e;

    public azz(bac bac2, azx azx2, String string) {
        this.b = bac2;
        this.c = azx2;
        this.d = string;
    }

    public void a(int n2) {
        if (this.c.c().b()) {
            throw new IllegalStateException("Cannot modify read-only score");
        }
        this.c(this.c() + n2);
    }

    public void b(int n2) {
        if (this.c.c().b()) {
            throw new IllegalStateException("Cannot modify read-only score");
        }
        this.c(this.c() - n2);
    }

    public void a() {
        if (this.c.c().b()) {
            throw new IllegalStateException("Cannot modify read-only score");
        }
        this.a(1);
    }

    public int c() {
        return this.e;
    }

    public void c(int n2) {
        int n3 = this.e;
        this.e = n2;
        if (n3 != n2) {
            this.f().a(this);
        }
    }

    public azx d() {
        return this.c;
    }

    public String e() {
        return this.d;
    }

    public bac f() {
        return this.b;
    }

    public void a(List list) {
        this.c(this.c.c().a(list));
    }
}

