/*
 * Decompiled with CFR 0.152.
 */
public class uf
extends ui {
    private sw a;

    public uf(sw sw2) {
        this.a = sw2;
        this.a(4);
        sw2.m().e(true);
    }

    @Override
    public boolean a() {
        return this.a.M() || this.a.P();
    }

    @Override
    public void e() {
        if (this.a.aI().nextFloat() < 0.8f) {
            this.a.l().a();
        }
    }
}

