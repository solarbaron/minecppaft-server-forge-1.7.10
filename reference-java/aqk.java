/*
 * Decompiled with CFR 0.152.
 */
import java.io.DataInputStream;
import java.io.DataOutput;
import java.io.DataOutputStream;
import java.io.File;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class aqk
implements aqc,
azs {
    private static final Logger a = LogManager.getLogger();
    private List b = new ArrayList();
    private Set c = new HashSet();
    private Object d = new Object();
    private final File e;

    public aqk(File file) {
        this.e = file;
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    @Override
    public apx a(ahb ahb2, int n2, int n3) {
        dh dh2 = null;
        agu agu2 = new agu(n2, n3);
        Object object = this.d;
        synchronized (object) {
            if (this.c.contains(agu2)) {
                for (int i2 = 0; i2 < this.b.size(); ++i2) {
                    if (!((aql)this.b.get((int)i2)).a.equals(agu2)) continue;
                    dh2 = ((aql)this.b.get((int)i2)).b;
                    break;
                }
            }
        }
        if (dh2 == null) {
            object = aqj.c(this.e, n2, n3);
            if (object != null) {
                dh2 = du.a((DataInputStream)object);
            } else {
                return null;
            }
        }
        return this.a(ahb2, n2, n3, dh2);
    }

    protected apx a(ahb ahb2, int n2, int n3, dh dh2) {
        if (!dh2.b("Level", 10)) {
            a.error("Chunk file at " + n2 + "," + n3 + " is missing level data, skipping");
            return null;
        }
        if (!dh2.m("Level").b("Sections", 9)) {
            a.error("Chunk file at " + n2 + "," + n3 + " is missing block data, skipping");
            return null;
        }
        apx apx2 = this.a(ahb2, dh2.m("Level"));
        if (!apx2.a(n2, n3)) {
            a.error("Chunk file at " + n2 + "," + n3 + " is in the wrong location; relocating. (Expected " + n2 + ", " + n3 + ", got " + apx2.g + ", " + apx2.h + ")");
            dh2.a("xPos", n2);
            dh2.a("zPos", n3);
            apx2 = this.a(ahb2, dh2.m("Level"));
        }
        return apx2;
    }

    @Override
    public void a(ahb ahb2, apx apx2) {
        ahb2.G();
        try {
            dh dh2 = new dh();
            dh dh3 = new dh();
            dh2.a("Level", dh3);
            this.a(apx2, ahb2, dh3);
            this.a(apx2.l(), dh2);
        }
        catch (Exception exception) {
            exception.printStackTrace();
        }
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    protected void a(agu agu2, dh dh2) {
        Object object = this.d;
        synchronized (object) {
            if (this.c.contains(agu2)) {
                for (int i2 = 0; i2 < this.b.size(); ++i2) {
                    if (!((aql)this.b.get((int)i2)).a.equals(agu2)) continue;
                    this.b.set(i2, new aql(agu2, dh2));
                    return;
                }
            }
            this.b.add(new aql(agu2, dh2));
            this.c.add(agu2);
            azr.a.a(this);
        }
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    @Override
    public boolean c() {
        aql aql2 = null;
        Object object = this.d;
        synchronized (object) {
            if (this.b.isEmpty()) {
                return false;
            }
            aql2 = (aql)this.b.remove(0);
            this.c.remove(aql2.a);
        }
        if (aql2 != null) {
            try {
                this.a(aql2);
            }
            catch (Exception exception) {
                exception.printStackTrace();
            }
        }
        return true;
    }

    private void a(aql aql2) {
        DataOutputStream dataOutputStream = aqj.d(this.e, aql2.a.a, aql2.a.b);
        du.a(aql2.b, (DataOutput)dataOutputStream);
        dataOutputStream.close();
    }

    @Override
    public void b(ahb ahb2, apx apx2) {
    }

    @Override
    public void a() {
    }

    @Override
    public void b() {
        while (this.c()) {
        }
    }

    private void a(apx apx2, ahb ahb2, dh dh2) {
        dh dh3;
        dh2.a("V", (byte)1);
        dh2.a("xPos", apx2.g);
        dh2.a("zPos", apx2.h);
        dh2.a("LastUpdate", ahb2.I());
        dh2.a("HeightMap", apx2.f);
        dh2.a("TerrainPopulated", apx2.k);
        dh2.a("LightPopulated", apx2.l);
        dh2.a("InhabitedTime", apx2.s);
        apz[] apzArray = apx2.i();
        dq dq2 = new dq();
        boolean bl2 = !ahb2.t.g;
        for (apz object : apzArray) {
            if (object == null) continue;
            dh3 = new dh();
            dh3.a("Y", (byte)(object.d() >> 4 & 0xFF));
            dh3.a("Blocks", object.g());
            if (object.i() != null) {
                dh3.a("Add", object.i().a);
            }
            dh3.a("Data", object.j().a);
            dh3.a("BlockLight", object.k().a);
            if (bl2) {
                dh3.a("SkyLight", object.l().a);
            } else {
                dh3.a("SkyLight", new byte[object.k().a.length]);
            }
            dq2.a(dh3);
        }
        dh2.a("Sections", dq2);
        dh2.a("Biomes", apx2.m());
        apx2.o = false;
        dq dq3 = new dq();
        for (int i2 = 0; i2 < apx2.j.length; ++i2) {
            for (sa sa2 : apx2.j[i2]) {
                if (!sa2.d(dh3 = new dh())) continue;
                apx2.o = true;
                dq3.a(dh3);
            }
        }
        dh2.a("Entities", dq3);
        dq dq4 = new dq();
        for (aor aor2 : apx2.i.values()) {
            dh3 = new dh();
            aor2.b(dh3);
            dq4.a(dh3);
        }
        dh2.a("TileEntities", dq4);
        List list = ahb2.a(apx2, false);
        if (list != null) {
            long l2 = ahb2.I();
            dq dq5 = new dq();
            Iterator iterator = list.iterator();
            while (iterator.hasNext()) {
                ahs ahs2 = (ahs)iterator.next();
                dh dh4 = new dh();
                dh4.a("i", aji.b(ahs2.a()));
                dh4.a("x", ahs2.a);
                dh4.a("y", ahs2.b);
                dh4.a("z", ahs2.c);
                dh4.a("t", (int)(ahs2.d - l2));
                dh4.a("p", ahs2.e);
                dq5.a(dh4);
            }
            dh2.a("TileTicks", dq5);
        }
    }

    private apx a(ahb ahb2, dh dh2) {
        dq dq2;
        dq dq3;
        Object object;
        dq dq4;
        Object object2;
        int n2 = dh2.f("xPos");
        int n3 = dh2.f("zPos");
        apx apx2 = new apx(ahb2, n2, n3);
        apx2.f = dh2.l("HeightMap");
        apx2.k = dh2.n("TerrainPopulated");
        apx2.l = dh2.n("LightPopulated");
        apx2.s = dh2.g("InhabitedTime");
        dq dq5 = dh2.c("Sections", 10);
        int n4 = 16;
        apz[] apzArray = new apz[n4];
        boolean bl2 = !ahb2.t.g;
        for (int i2 = 0; i2 < dq5.c(); ++i2) {
            dh dh3 = dq5.b(i2);
            byte by2 = dh3.d("Y");
            object2 = new apz(by2 << 4, bl2);
            ((apz)object2).a(dh3.k("Blocks"));
            if (dh3.b("Add", 7)) {
                ((apz)object2).a(new apv(dh3.k("Add"), 4));
            }
            ((apz)object2).b(new apv(dh3.k("Data"), 4));
            ((apz)object2).c(new apv(dh3.k("BlockLight"), 4));
            if (bl2) {
                ((apz)object2).d(new apv(dh3.k("SkyLight"), 4));
            }
            ((apz)object2).e();
            apzArray[by2] = object2;
        }
        apx2.a(apzArray);
        if (dh2.b("Biomes", 7)) {
            apx2.a(dh2.k("Biomes"));
        }
        if ((dq4 = dh2.c("Entities", 10)) != null) {
            for (int i3 = 0; i3 < dq4.c(); ++i3) {
                dh dh4 = dq4.b(i3);
                object2 = sg.a(dh4, ahb2);
                apx2.o = true;
                if (object2 == null) continue;
                apx2.a((sa)object2);
                object = object2;
                dh dh5 = dh4;
                while (dh5.b("Riding", 10)) {
                    sa sa2 = sg.a(dh5.m("Riding"), ahb2);
                    if (sa2 != null) {
                        apx2.a(sa2);
                        ((sa)object).a(sa2);
                    }
                    object = sa2;
                    dh5 = dh5.m("Riding");
                }
            }
        }
        if ((dq3 = dh2.c("TileEntities", 10)) != null) {
            for (int i4 = 0; i4 < dq3.c(); ++i4) {
                object2 = dq3.b(i4);
                object = aor.c((dh)object2);
                if (object == null) continue;
                apx2.a((aor)object);
            }
        }
        if (dh2.b("TileTicks", 9) && (dq2 = dh2.c("TileTicks", 10)) != null) {
            for (int i5 = 0; i5 < dq2.c(); ++i5) {
                object = dq2.b(i5);
                ahb2.b(((dh)object).f("x"), ((dh)object).f("y"), ((dh)object).f("z"), aji.e(((dh)object).f("i")), ((dh)object).f("t"), ((dh)object).f("p"));
            }
        }
        return apx2;
    }
}

