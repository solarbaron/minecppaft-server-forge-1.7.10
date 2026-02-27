/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;

public class tb
extends ss {
    public tc e;

    public tb(ahb ahb2) {
        super(ahb2);
    }

    public tb(ahb ahb2, int n2, int n3, int n4, int n5) {
        super(ahb2, n2, n3, n4, n5);
        ArrayList<tc> arrayList = new ArrayList<tc>();
        tc[] tcArray = tc.values();
        int n6 = tcArray.length;
        for (int i2 = 0; i2 < n6; ++i2) {
            tc tc2;
            this.e = tc2 = tcArray[i2];
            this.a(n5);
            if (!this.e()) continue;
            arrayList.add(tc2);
        }
        if (!arrayList.isEmpty()) {
            this.e = (tc)((Object)arrayList.get(this.Z.nextInt(arrayList.size())));
        }
        this.a(n5);
    }

    @Override
    public void b(dh dh2) {
        dh2.a("Motive", this.e.B);
        super.b(dh2);
    }

    @Override
    public void a(dh dh2) {
        String string = dh2.j("Motive");
        for (tc tc2 : tc.values()) {
            if (!tc2.B.equals(string)) continue;
            this.e = tc2;
        }
        if (this.e == null) {
            this.e = tc.a;
        }
        super.a(dh2);
    }

    @Override
    public int f() {
        return this.e.C;
    }

    @Override
    public int i() {
        return this.e.D;
    }

    @Override
    public void b(sa sa2) {
        if (sa2 instanceof yz) {
            yz yz2 = (yz)sa2;
            if (yz2.bE.d) {
                return;
            }
        }
        this.a(new add(ade.an), 0.0f);
    }
}

