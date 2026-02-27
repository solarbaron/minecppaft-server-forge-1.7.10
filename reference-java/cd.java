/*
 * Decompiled with CFR 0.152.
 */
public class cd
extends RuntimeException {
    private Object[] a;

    public cd(String string, Object ... objectArray) {
        super(string);
        this.a = objectArray;
    }

    public Object[] a() {
        return this.a;
    }
}

