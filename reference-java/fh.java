/*
 * Decompiled with CFR 0.152.
 */
public class fh {
    private final fi a;
    private final String b;

    public fh(fi fi2, String string) {
        this.a = fi2;
        this.b = string;
    }

    public fi a() {
        return this.a;
    }

    public String b() {
        return this.b;
    }

    public boolean equals(Object object) {
        if (this == object) {
            return true;
        }
        if (object == null || this.getClass() != object.getClass()) {
            return false;
        }
        fh fh2 = (fh)object;
        if (this.a != fh2.a) {
            return false;
        }
        return !(this.b != null ? !this.b.equals(fh2.b) : fh2.b != null);
    }

    public String toString() {
        return "ClickEvent{action=" + (Object)((Object)this.a) + ", value='" + this.b + '\'' + '}';
    }

    public int hashCode() {
        int n2 = this.a.hashCode();
        n2 = 31 * n2 + (this.b != null ? this.b.hashCode() : 0);
        return n2;
    }
}

