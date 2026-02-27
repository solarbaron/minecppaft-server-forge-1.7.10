/*
 * Decompiled with CFR 0.152.
 */
public class apl
extends aor {
    public byte a;
    public boolean i;

    @Override
    public void b(dh dh2) {
        super.b(dh2);
        dh2.a("note", this.a);
    }

    @Override
    public void a(dh dh2) {
        super.a(dh2);
        this.a = dh2.d("note");
        if (this.a < 0) {
            this.a = 0;
        }
        if (this.a > 24) {
            this.a = (byte)24;
        }
    }

    public void a() {
        this.a = (byte)((this.a + 1) % 25);
        this.e();
    }

    public void a(ahb ahb2, int n2, int n3, int n4) {
        if (ahb2.a(n2, n3 + 1, n4).o() != awt.a) {
            return;
        }
        awt awt2 = ahb2.a(n2, n3 - 1, n4).o();
        int n5 = 0;
        if (awt2 == awt.e) {
            n5 = 1;
        }
        if (awt2 == awt.p) {
            n5 = 2;
        }
        if (awt2 == awt.s) {
            n5 = 3;
        }
        if (awt2 == awt.d) {
            n5 = 4;
        }
        ahb2.c(n2, n3, n4, ajn.B, n5, (int)this.a);
    }
}

