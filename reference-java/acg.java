/*
 * Decompiled with CFR 0.152.
 */
import com.google.common.collect.Multimap;
import java.util.Set;

public class acg
extends adb {
    private Set c;
    protected float a = 4.0f;
    private float d;
    protected adc b;

    protected acg(float f2, adc adc2, Set set) {
        this.b = adc2;
        this.c = set;
        this.h = 1;
        this.f(adc2.a());
        this.a = adc2.b();
        this.d = f2 + adc2.c();
        this.a(abt.i);
    }

    @Override
    public float a(add add2, aji aji2) {
        return this.c.contains(aji2) ? this.a : 1.0f;
    }

    @Override
    public boolean a(add add2, sv sv2, sv sv3) {
        add2.a(2, sv3);
        return true;
    }

    @Override
    public boolean a(add add2, ahb ahb2, aji aji2, int n2, int n3, int n4, sv sv2) {
        if ((double)aji2.f(ahb2, n2, n3, n4) != 0.0) {
            add2.a(1, sv2);
        }
        return true;
    }

    public adc i() {
        return this.b;
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
        multimap.put(yj.e.a(), new tj(f, "Tool modifier", this.d, 0));
        return multimap;
    }
}

