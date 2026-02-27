/*
 * Decompiled with CFR 0.152.
 */
public class fl {
    private final fm a;
    private final fj b;

    public fl(fm fm2, fj fj2) {
        this.a = fm2;
        this.b = fj2;
    }

    public fm a() {
        return this.a;
    }

    public fj b() {
        return this.b;
    }

    public boolean equals(Object object) {
        if (this == object) {
            return true;
        }
        if (object == null || this.getClass() != object.getClass()) {
            return false;
        }
        fl fl2 = (fl)object;
        if (this.a != fl2.a) {
            return false;
        }
        return !(this.b != null ? !this.b.equals(fl2.b) : fl2.b != null);
    }

    public String toString() {
        return "HoverEvent{action=" + (Object)((Object)this.a) + ", value='" + this.b + '\'' + '}';
    }

    public int hashCode() {
        int n2 = this.a.hashCode();
        n2 = 31 * n2 + (this.b != null ? this.b.hashCode() : 0);
        return n2;
    }
}

