/*
 * Decompiled with CFR 0.152.
 */
public class avj
extends ayl {
    private dh a = new dh();

    public avj(String string) {
        super(string);
    }

    @Override
    public void a(dh dh2) {
        this.a = dh2.m("Features");
    }

    @Override
    public void b(dh dh2) {
        dh2.a("Features", this.a);
    }

    public void a(dh dh2, int n2, int n3) {
        this.a.a(avj.b(n2, n3), dh2);
    }

    public static String b(int n2, int n3) {
        return "[" + n2 + "," + n3 + "]";
    }

    public dh a() {
        return this.a;
    }
}

