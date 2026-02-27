/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Random;

public class aqu
implements apu {
    private ahb a;
    private Random b;
    private final aji[] c = new aji[256];
    private final byte[] d = new byte[256];
    private final ass e;
    private final List f = new ArrayList();
    private final boolean g;
    private final boolean h;
    private arx i;
    private arx j;

    public aqu(ahb ahb2, long l2, boolean bl2, String string) {
        this.a = ahb2;
        this.b = new Random(l2);
        this.e = ass.a(string);
        if (bl2) {
            Map map = this.e.b();
            if (map.containsKey("village")) {
                Object object = (Map)map.get("village");
                if (!object.containsKey("size")) {
                    object.put("size", "1");
                }
                this.f.add(new avn((Map)object));
            }
            if (map.containsKey("biome_1")) {
                this.f.add(new atx((Map)map.get("biome_1")));
            }
            if (map.containsKey("mineshaft")) {
                this.f.add(new asw((Map)map.get("mineshaft")));
            }
            if (map.containsKey("stronghold")) {
                this.f.add(new aug((Map)map.get("stronghold")));
            }
        }
        this.g = this.e.b().containsKey("decoration");
        if (this.e.b().containsKey("lake")) {
            this.i = new arx(ajn.j);
        }
        if (this.e.b().containsKey("lava_lake")) {
            this.j = new arx(ajn.l);
        }
        this.h = this.e.b().containsKey("dungeon");
        for (Object object : this.e.c()) {
            for (int i2 = ((ast)object).d(); i2 < ((ast)object).d() + ((ast)object).a(); ++i2) {
                this.c[i2] = ((ast)object).b();
                this.d[i2] = (byte)((ast)object).c();
            }
        }
    }

    @Override
    public apx c(int n2, int n3) {
        return this.d(n2, n3);
    }

    @Override
    public apx d(int n2, int n3) {
        int n4;
        Object object;
        apx apx2 = new apx(this.a, n2, n3);
        for (int i2 = 0; i2 < this.c.length; ++i2) {
            object = this.c[i2];
            if (object == null) continue;
            n4 = i2 >> 4;
            Object object2 = apx2.i()[n4];
            if (object2 == null) {
                object2 = new apz(i2, !this.a.t.g);
                apx2.i()[n4] = object2;
            }
            for (int i3 = 0; i3 < 16; ++i3) {
                for (int i4 = 0; i4 < 16; ++i4) {
                    ((apz)object2).a(i3, i2 & 0xF, i4, (aji)object);
                    ((apz)object2).a(i3, i2 & 0xF, i4, this.d[i2]);
                }
            }
        }
        apx2.b();
        ahu[] ahuArray = this.a.v().b(null, n2 * 16, n3 * 16, 16, 16);
        object = apx2.m();
        for (n4 = 0; n4 < ((Object)object).length; ++n4) {
            object[n4] = (byte)ahuArray[n4].ay;
        }
        for (Object object2 : this.f) {
            ((aqx)object2).a(this, this.a, n2, n3, null);
        }
        apx2.b();
        return apx2;
    }

    @Override
    public boolean a(int n2, int n3) {
        return true;
    }

    @Override
    public void a(apu apu2, int n2, int n3) {
        int n4;
        int n5 = n2 * 16;
        int n6 = n3 * 16;
        ahu ahu2 = this.a.a(n5 + 16, n6 + 16);
        boolean bl2 = false;
        this.b.setSeed(this.a.H());
        long l2 = this.b.nextLong() / 2L * 2L + 1L;
        long l3 = this.b.nextLong() / 2L * 2L + 1L;
        this.b.setSeed((long)n2 * l2 + (long)n3 * l3 ^ this.a.H());
        for (ave ave2 : this.f) {
            n4 = ave2.a(this.a, this.b, n2, n3);
            if (!(ave2 instanceof avn)) continue;
            bl2 |= n4;
        }
        if (this.i != null && !bl2 && this.b.nextInt(4) == 0) {
            int n7 = n5 + this.b.nextInt(16) + 8;
            int n8 = this.b.nextInt(256);
            n4 = n6 + this.b.nextInt(16) + 8;
            this.i.a(this.a, this.b, n7, n8, n4);
        }
        if (this.j != null && !bl2 && this.b.nextInt(8) == 0) {
            int n9 = n5 + this.b.nextInt(16) + 8;
            int n10 = this.b.nextInt(this.b.nextInt(248) + 8);
            n4 = n6 + this.b.nextInt(16) + 8;
            if (n10 < 63 || this.b.nextInt(10) == 0) {
                this.j.a(this.a, this.b, n9, n10, n4);
            }
        }
        if (this.h) {
            for (int i2 = 0; i2 < 8; ++i2) {
                int n11 = n5 + this.b.nextInt(16) + 8;
                n4 = this.b.nextInt(256);
                int n12 = n6 + this.b.nextInt(16) + 8;
                new asd().a(this.a, this.b, n11, n4, n12);
            }
        }
        if (this.g) {
            ahu2.a(this.a, this.b, n5, n6);
        }
    }

    @Override
    public boolean a(boolean bl2, qk qk2) {
        return true;
    }

    @Override
    public void c() {
    }

    @Override
    public boolean d() {
        return false;
    }

    @Override
    public boolean e() {
        return true;
    }

    @Override
    public String f() {
        return "FlatLevelSource";
    }

    @Override
    public List a(sx sx2, int n2, int n3, int n4) {
        ahu ahu2 = this.a.a(n2, n4);
        return ahu2.a(sx2);
    }

    @Override
    public agt a(ahb ahb2, String string, int n2, int n3, int n4) {
        if ("Stronghold".equals(string)) {
            for (ave ave2 : this.f) {
                if (!(ave2 instanceof aug)) continue;
                return ave2.a(ahb2, n2, n3, n4);
            }
        }
        return null;
    }

    @Override
    public int g() {
        return 0;
    }

    @Override
    public void e(int n2, int n3) {
        for (ave ave2 : this.f) {
            ave2.a(this, this.a, n2, n3, null);
        }
    }
}

