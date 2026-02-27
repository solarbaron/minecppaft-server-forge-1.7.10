/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;

public class baf
implements bah {
    private final String g;

    public baf(String string) {
        this.g = string;
        bah.a.put(string, this);
    }

    @Override
    public String a() {
        return this.g;
    }

    @Override
    public int a(List list) {
        return 0;
    }

    @Override
    public boolean b() {
        return false;
    }
}

