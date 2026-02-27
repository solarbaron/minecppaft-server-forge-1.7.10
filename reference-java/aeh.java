/*
 * Decompiled with CFR 0.152.
 */
import com.google.common.collect.Multimap;

public class aeh
extends adb {
    private float a;
    private final adc b;

    public aeh(adc adc2) {
        this.b = adc2;
        this.h = 1;
        this.f(adc2.a());
        this.a(abt.j);
        this.a = 4.0f + adc2.c();
    }

    public float i() {
        return this.b.c();
    }

    @Override
    public float a(add add2, aji aji2) {
        if (aji2 == ajn.G) {
            return 15.0f;
        }
        awt awt2 = aji2.o();
        if (awt2 == awt.k || awt2 == awt.l || awt2 == awt.v || awt2 == awt.j || awt2 == awt.C) {
            return 1.5f;
        }
        return 1.0f;
    }

    @Override
    public boolean a(add add2, sv sv2, sv sv3) {
        add2.a(1, sv3);
        return true;
    }

    @Override
    public boolean a(add add2, ahb ahb2, aji aji2, int n2, int n3, int n4, sv sv2) {
        if ((double)aji2.f(ahb2, n2, n3, n4) != 0.0) {
            add2.a(2, sv2);
        }
        return true;
    }

    @Override
    public aei d(add add2) {
        return aei.d;
    }

    @Override
    public int d_(add add2) {
        return 72000;
    }

    @Override
    public add a(add add2, ahb ahb2, yz yz2) {
        yz2.a(add2, this.d_(add2));
        return add2;
    }

    @Override
    public boolean b(aji aji2) {
        return aji2 == ajn.G;
    }

    @Override
    public int c() {
        return this.b.e();
    }

    public String j() {
        return this.b.toString();
    }

    @Override
    public boolean a(add add2, add add3) {
        if (this.b.f() == add3.b()) {
            return true;
        }
        return super.a(add2, add3);
    }

    @Override
    public Multimap k() {
        Multimap multimap = super.k();
        multimap.put(yj.e.a(), new tj(f, "Weapon modifier", this.a, 0));
        return multimap;
    }
}

