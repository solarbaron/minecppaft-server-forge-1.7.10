/*
 * Decompiled with CFR 0.152.
 */
public class aav
extends rh {
    private ape a;

    public aav() {
        super("container.enderchest", false, 27);
    }

    public void a(ape ape2) {
        this.a = ape2;
    }

    public void a(dq dq2) {
        int n2;
        for (n2 = 0; n2 < this.a(); ++n2) {
            this.a(n2, null);
        }
        for (n2 = 0; n2 < dq2.c(); ++n2) {
            dh dh2 = dq2.b(n2);
            int n3 = dh2.d("Slot") & 0xFF;
            if (n3 < 0 || n3 >= this.a()) continue;
            this.a(n3, add.a(dh2));
        }
    }

    public dq h() {
        dq dq2 = new dq();
        for (int i2 = 0; i2 < this.a(); ++i2) {
            add add2 = this.a(i2);
            if (add2 == null) continue;
            dh dh2 = new dh();
            dh2.a("Slot", (byte)i2);
            add2.b(dh2);
            dq2.a(dh2);
        }
        return dq2;
    }

    @Override
    public boolean a(yz yz2) {
        if (this.a != null && !this.a.a(yz2)) {
            return false;
        }
        return super.a(yz2);
    }

    @Override
    public void f() {
        if (this.a != null) {
            this.a.a();
        }
        super.f();
    }

    @Override
    public void l_() {
        if (this.a != null) {
            this.a.b();
        }
        super.l_();
        this.a = null;
    }
}

