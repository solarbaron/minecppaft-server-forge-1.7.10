/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;

public class hw
extends ft {
    private int a;
    private List b;

    public hw() {
    }

    public hw(int n2, te te2, boolean bl2) {
        this.a = n2;
        this.b = bl2 ? te2.c() : te2.b();
    }

    @Override
    public void a(et et2) {
        this.a = et2.readInt();
        this.b = te.b(et2);
    }

    @Override
    public void b(et et2) {
        et2.writeInt(this.a);
        te.a(this.b, et2);
    }

    public void a(fv fv2) {
        fv2.a(this);
    }
}

