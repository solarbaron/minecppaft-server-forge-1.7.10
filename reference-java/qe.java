/*
 * Decompiled with CFR 0.152.
 */
class qe {
    final long a;
    Object b;
    qe c;
    final int d;

    qe(int n2, long l2, Object object, qe qe2) {
        this.b = object;
        this.c = qe2;
        this.a = l2;
        this.d = n2;
    }

    public final long a() {
        return this.a;
    }

    public final Object b() {
        return this.b;
    }

    public final boolean equals(Object object) {
        Object object2;
        Object object3;
        Long l2;
        if (!(object instanceof qe)) {
            return false;
        }
        qe qe2 = (qe)object;
        Long l3 = this.a();
        return (l3 == (l2 = Long.valueOf(qe2.a())) || l3 != null && ((Object)l3).equals(l2)) && ((object3 = this.b()) == (object2 = qe2.b()) || object3 != null && object3.equals(object2));
    }

    public final int hashCode() {
        return qd.f(this.a);
    }

    public final String toString() {
        return this.a() + "=" + this.b();
    }
}

