/*
 * Decompiled with CFR 0.152.
 */
public class ahx
extends qw {
    public Class b;
    public int c;
    public int d;

    public ahx(Class clazz, int n2, int n3, int n4) {
        super(n2);
        this.b = clazz;
        this.c = n3;
        this.d = n4;
    }

    public String toString() {
        return this.b.getSimpleName() + "*(" + this.c + "-" + this.d + "):" + this.a;
    }
}

