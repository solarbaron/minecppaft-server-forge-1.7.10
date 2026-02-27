/*
 * Decompiled with CFR 0.152.
 */
public class ast {
    private aji a;
    private int b = 1;
    private int c;
    private int d;

    public ast(int n2, aji aji2) {
        this.b = n2;
        this.a = aji2;
    }

    public ast(int n2, aji aji2, int n3) {
        this(n2, aji2);
        this.c = n3;
    }

    public int a() {
        return this.b;
    }

    public aji b() {
        return this.a;
    }

    public int c() {
        return this.c;
    }

    public int d() {
        return this.d;
    }

    public void c(int n2) {
        this.d = n2;
    }

    public String toString() {
        String string = Integer.toString(aji.b(this.a));
        if (this.b > 1) {
            string = this.b + "x" + string;
        }
        if (this.c > 0) {
            string = string + ":" + this.c;
        }
        return string;
    }
}

