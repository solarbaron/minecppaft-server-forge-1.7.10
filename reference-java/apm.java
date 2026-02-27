/*
 * Decompiled with CFR 0.152.
 */
public class apm
extends aor {
    public String[] a = new String[]{"", "", "", ""};
    public int i = -1;
    private boolean j = true;
    private yz k;

    @Override
    public void b(dh dh2) {
        super.b(dh2);
        dh2.a("Text1", this.a[0]);
        dh2.a("Text2", this.a[1]);
        dh2.a("Text3", this.a[2]);
        dh2.a("Text4", this.a[3]);
    }

    @Override
    public void a(dh dh2) {
        this.j = false;
        super.a(dh2);
        for (int i2 = 0; i2 < 4; ++i2) {
            this.a[i2] = dh2.j("Text" + (i2 + 1));
            if (this.a[i2].length() <= 15) continue;
            this.a[i2] = this.a[i2].substring(0, 15);
        }
    }

    @Override
    public ft m() {
        String[] stringArray = new String[4];
        System.arraycopy(this.a, 0, stringArray, 0, 4);
        return new ii(this.c, this.d, this.e, stringArray);
    }

    public boolean a() {
        return this.j;
    }

    public void a(yz yz2) {
        this.k = yz2;
    }

    public yz b() {
        return this.k;
    }
}

