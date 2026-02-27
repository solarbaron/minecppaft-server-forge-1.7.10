/*
 * Decompiled with CFR 0.152.
 */
public class pb
extends ph {
    public final int a;
    public final int b;
    public final pb c;
    private final String k;
    public final add d;
    private boolean m;

    public pb(String string, String string2, int n2, int n3, adb adb2, pb pb2) {
        this(string, string2, n2, n3, new add(adb2), pb2);
    }

    public pb(String string, String string2, int n2, int n3, aji aji2, pb pb2) {
        this(string, string2, n2, n3, new add(aji2), pb2);
    }

    public pb(String string, String string2, int n2, int n3, add add2, pb pb2) {
        super(string, new fr("achievement." + string2, new Object[0]));
        this.d = add2;
        this.k = "achievement." + string2 + ".desc";
        this.a = n2;
        this.b = n3;
        if (n2 < pc.a) {
            pc.a = n2;
        }
        if (n3 < pc.b) {
            pc.b = n3;
        }
        if (n2 > pc.c) {
            pc.c = n2;
        }
        if (n3 > pc.d) {
            pc.d = n3;
        }
        this.c = pb2;
    }

    public pb a() {
        this.f = true;
        return this;
    }

    public pb b() {
        this.m = true;
        return this;
    }

    public pb c() {
        super.h();
        pc.e.add(this);
        return this;
    }

    @Override
    public boolean d() {
        return true;
    }

    @Override
    public fj e() {
        fj fj2 = super.e();
        fj2.b().a(this.g() ? a.f : a.k);
        return fj2;
    }

    public pb a(Class clazz) {
        return (pb)super.b(clazz);
    }

    public boolean g() {
        return this.m;
    }

    @Override
    public /* synthetic */ ph b(Class clazz) {
        return this.a(clazz);
    }

    @Override
    public /* synthetic */ ph h() {
        return this.c();
    }

    @Override
    public /* synthetic */ ph i() {
        return this.a();
    }
}

