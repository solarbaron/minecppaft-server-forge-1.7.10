/*
 * Decompiled with CFR 0.152.
 */
public class fs
extends IllegalArgumentException {
    public fs(fr fr2, String string) {
        super(String.format("Error parsing: %s: %s", fr2, string));
    }

    public fs(fr fr2, int n2) {
        super(String.format("Invalid index %d requested for %s", n2, fr2));
    }

    public fs(fr fr2, Throwable throwable) {
        super(String.format("Error while parsing: %s", fr2), throwable);
    }
}

