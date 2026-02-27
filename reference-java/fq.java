/*
 * Decompiled with CFR 0.152.
 */
public class fq
extends fe {
    private final String b;

    public fq(String string) {
        this.b = string;
    }

    public String g() {
        return this.b;
    }

    @Override
    public String e() {
        return this.b;
    }

    public fq h() {
        fq fq2 = new fq(this.b);
        fq2.a(this.b().l());
        for (fj fj2 : this.a()) {
            fq2.a(fj2.f());
        }
        return fq2;
    }

    @Override
    public boolean equals(Object object) {
        if (this == object) {
            return true;
        }
        if (object instanceof fq) {
            fq fq2 = (fq)object;
            return this.b.equals(fq2.g()) && super.equals(object);
        }
        return false;
    }

    @Override
    public String toString() {
        return "TextComponent{text='" + this.b + '\'' + ", siblings=" + this.a + ", style=" + this.b() + '}';
    }

    @Override
    public /* synthetic */ fj f() {
        return this.h();
    }
}

