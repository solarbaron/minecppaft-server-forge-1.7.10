/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class apb
extends aor
implements rb {
    private add[] i = new add[9];
    private Random j = new Random();
    protected String a;

    @Override
    public int a() {
        return 9;
    }

    @Override
    public add a(int n2) {
        return this.i[n2];
    }

    @Override
    public add a(int n2, int n3) {
        if (this.i[n2] != null) {
            if (this.i[n2].b <= n3) {
                add add2 = this.i[n2];
                this.i[n2] = null;
                this.e();
                return add2;
            }
            add add3 = this.i[n2].a(n3);
            if (this.i[n2].b == 0) {
                this.i[n2] = null;
            }
            this.e();
            return add3;
        }
        return null;
    }

    @Override
    public add a_(int n2) {
        if (this.i[n2] != null) {
            add add2 = this.i[n2];
            this.i[n2] = null;
            return add2;
        }
        return null;
    }

    public int i() {
        int n2 = -1;
        int n3 = 1;
        for (int i2 = 0; i2 < this.i.length; ++i2) {
            if (this.i[i2] == null || this.j.nextInt(n3++) != 0) continue;
            n2 = i2;
        }
        return n2;
    }

    @Override
    public void a(int n2, add add2) {
        this.i[n2] = add2;
        if (add2 != null && add2.b > this.d()) {
            add2.b = this.d();
        }
        this.e();
    }

    public int a(add add2) {
        for (int i2 = 0; i2 < this.i.length; ++i2) {
            if (this.i[i2] != null && this.i[i2].b() != null) continue;
            this.a(i2, add2);
            return i2;
        }
        return -1;
    }

    @Override
    public String b() {
        return this.k_() ? this.a : "container.dispenser";
    }

    public void a(String string) {
        this.a = string;
    }

    @Override
    public boolean k_() {
        return this.a != null;
    }

    @Override
    public void a(dh dh2) {
        super.a(dh2);
        dq dq2 = dh2.c("Items", 10);
        this.i = new add[this.a()];
        for (int i2 = 0; i2 < dq2.c(); ++i2) {
            dh dh3 = dq2.b(i2);
            int n2 = dh3.d("Slot") & 0xFF;
            if (n2 < 0 || n2 >= this.i.length) continue;
            this.i[n2] = add.a(dh3);
        }
        if (dh2.b("CustomName", 8)) {
            this.a = dh2.j("CustomName");
        }
    }

    @Override
    public void b(dh dh2) {
        super.b(dh2);
        dq dq2 = new dq();
        for (int i2 = 0; i2 < this.i.length; ++i2) {
            if (this.i[i2] == null) continue;
            dh dh3 = new dh();
            dh3.a("Slot", (byte)i2);
            this.i[i2].b(dh3);
            dq2.a(dh3);
        }
        dh2.a("Items", dq2);
        if (this.k_()) {
            dh2.a("CustomName", this.a);
        }
    }

    @Override
    public int d() {
        return 64;
    }

    @Override
    public boolean a(yz yz2) {
        if (this.b.o(this.c, this.d, this.e) != this) {
            return false;
        }
        return !(yz2.e((double)this.c + 0.5, (double)this.d + 0.5, (double)this.e + 0.5) > 64.0);
    }

    @Override
    public void f() {
    }

    @Override
    public void l_() {
    }

    @Override
    public boolean b(int n2, add add2) {
        return true;
    }
}

