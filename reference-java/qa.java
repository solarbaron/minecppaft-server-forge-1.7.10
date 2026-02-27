/*
 * Decompiled with CFR 0.152.
 */
class qa {
    final int a;
    Object b;
    qa c;
    final int d;

    qa(int n2, int n3, Object object, qa qa2) {
        this.b = object;
        this.c = qa2;
        this.a = n3;
        this.d = n2;
    }

    public final int a() {
        return this.a;
    }

    public final Object b() {
        return this.b;
    }

    public final boolean equals(Object object) {
        Object object2;
        Object object3;
        Integer n2;
        if (!(object instanceof qa)) {
            return false;
        }
        qa qa2 = (qa)object;
        Integer n3 = this.a();
        return (n3 == (n2 = Integer.valueOf(qa2.a())) || n3 != null && ((Object)n3).equals(n2)) && ((object3 = this.b()) == (object2 = qa2.b()) || object3 != null && object3.equals(object2));
    }

    public final int hashCode() {
        return pz.f(this.a);
    }

    public final String toString() {
        return this.a() + "=" + this.b();
    }
}

