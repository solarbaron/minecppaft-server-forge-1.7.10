/*
 * Decompiled with CFR 0.152.
 */
public class aqf {
    public static aqg a(dh dh2) {
        int n2 = dh2.f("xPos");
        int n3 = dh2.f("zPos");
        aqg aqg2 = new aqg(n2, n3);
        aqg2.g = dh2.k("Blocks");
        aqg2.f = new aqa(dh2.k("Data"), 7);
        aqg2.e = new aqa(dh2.k("SkyLight"), 7);
        aqg2.d = new aqa(dh2.k("BlockLight"), 7);
        aqg2.c = dh2.k("HeightMap");
        aqg2.b = dh2.n("TerrainPopulated");
        aqg2.h = dh2.c("Entities", 10);
        aqg2.i = dh2.c("TileEntities", 10);
        aqg2.j = dh2.c("TileTicks", 10);
        try {
            aqg2.a = dh2.g("LastUpdate");
        }
        catch (ClassCastException classCastException) {
            aqg2.a = dh2.f("LastUpdate");
        }
        return aqg2;
    }

    public static void a(aqg aqg2, dh dh2, aib aib2) {
        int n2;
        int n3;
        dh2.a("xPos", aqg2.k);
        dh2.a("zPos", aqg2.l);
        dh2.a("LastUpdate", aqg2.a);
        int[] nArray = new int[aqg2.c.length];
        for (int i2 = 0; i2 < aqg2.c.length; ++i2) {
            nArray[i2] = aqg2.c[i2];
        }
        dh2.a("HeightMap", nArray);
        dh2.a("TerrainPopulated", aqg2.b);
        dq dq2 = new dq();
        for (int i3 = 0; i3 < 8; ++i3) {
            int n4;
            n3 = 1;
            for (n2 = 0; n2 < 16 && n3 != 0; ++n2) {
                block3: for (int i4 = 0; i4 < 16 && n3 != 0; ++i4) {
                    for (int i5 = 0; i5 < 16; ++i5) {
                        int n5 = n2 << 11 | i5 << 7 | i4 + (i3 << 4);
                        n4 = aqg2.g[n5];
                        if (n4 == 0) continue;
                        n3 = 0;
                        continue block3;
                    }
                }
            }
            if (n3 != 0) continue;
            byte[] byArray = new byte[4096];
            apv apv2 = new apv(byArray.length, 4);
            apv apv3 = new apv(byArray.length, 4);
            apv apv4 = new apv(byArray.length, 4);
            for (n4 = 0; n4 < 16; ++n4) {
                for (int i6 = 0; i6 < 16; ++i6) {
                    for (int i7 = 0; i7 < 16; ++i7) {
                        int n6 = n4 << 11 | i7 << 7 | i6 + (i3 << 4);
                        byte by2 = aqg2.g[n6];
                        byArray[i6 << 8 | i7 << 4 | n4] = (byte)(by2 & 0xFF);
                        apv2.a(n4, i6, i7, aqg2.f.a(n4, i6 + (i3 << 4), i7));
                        apv3.a(n4, i6, i7, aqg2.e.a(n4, i6 + (i3 << 4), i7));
                        apv4.a(n4, i6, i7, aqg2.d.a(n4, i6 + (i3 << 4), i7));
                    }
                }
            }
            dh dh3 = new dh();
            dh3.a("Y", (byte)(i3 & 0xFF));
            dh3.a("Blocks", byArray);
            dh3.a("Data", apv2.a);
            dh3.a("SkyLight", apv3.a);
            dh3.a("BlockLight", apv4.a);
            dq2.a(dh3);
        }
        dh2.a("Sections", dq2);
        byte[] byArray = new byte[256];
        for (n3 = 0; n3 < 16; ++n3) {
            for (n2 = 0; n2 < 16; ++n2) {
                byArray[n2 << 4 | n3] = (byte)(aib2.a((int)(aqg2.k << 4 | n3), (int)(aqg2.l << 4 | n2)).ay & 0xFF);
            }
        }
        dh2.a("Biomes", byArray);
        dh2.a("Entities", aqg2.h);
        dh2.a("TileEntities", aqg2.i);
        if (aqg2.j != null) {
            dh2.a("TileTicks", aqg2.j);
        }
    }
}

