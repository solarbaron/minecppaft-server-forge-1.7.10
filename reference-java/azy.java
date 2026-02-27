/*
 * Decompiled with CFR 0.152.
 */
import java.util.Collection;
import java.util.HashSet;
import java.util.Set;

public class azy
extends bae {
    private final bac a;
    private final String b;
    private final Set c = new HashSet();
    private String d;
    private String e = "";
    private String f = "";
    private boolean g = true;
    private boolean h = true;

    public azy(bac bac2, String string) {
        this.a = bac2;
        this.b = string;
        this.d = string;
    }

    @Override
    public String b() {
        return this.b;
    }

    public String c() {
        return this.d;
    }

    public void a(String string) {
        if (string == null) {
            throw new IllegalArgumentException("Name cannot be null");
        }
        this.d = string;
        this.a.b(this);
    }

    public Collection d() {
        return this.c;
    }

    public String e() {
        return this.e;
    }

    public void b(String string) {
        if (string == null) {
            throw new IllegalArgumentException("Prefix cannot be null");
        }
        this.e = string;
        this.a.b(this);
    }

    public String f() {
        return this.f;
    }

    public void c(String string) {
        if (string == null) {
            throw new IllegalArgumentException("Suffix cannot be null");
        }
        this.f = string;
        this.a.b(this);
    }

    @Override
    public String d(String string) {
        return this.e() + string + this.f();
    }

    public static String a(bae bae2, String string) {
        if (bae2 == null) {
            return string;
        }
        return bae2.d(string);
    }

    @Override
    public boolean g() {
        return this.g;
    }

    public void a(boolean bl2) {
        this.g = bl2;
        this.a.b(this);
    }

    public boolean h() {
        return this.h;
    }

    public void b(boolean bl2) {
        this.h = bl2;
        this.a.b(this);
    }

    public int i() {
        int n2 = 0;
        if (this.g()) {
            n2 |= 1;
        }
        if (this.h()) {
            n2 |= 2;
        }
        return n2;
    }
}

