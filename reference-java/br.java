/*
 * Decompiled with CFR 0.152.
 */
import com.google.common.collect.Lists;
import com.google.common.collect.Maps;
import com.google.common.collect.Sets;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Random;
import net.minecraft.server.MinecraftServer;

public class br
extends y {
    @Override
    public String c() {
        return "spreadplayers";
    }

    @Override
    public int a() {
        return 2;
    }

    @Override
    public String c(ac ac2) {
        return "commands.spreadplayers.usage";
    }

    @Override
    public void b(ac ac2, String[] stringArray) {
        if (stringArray.length < 6) {
            throw new ci("commands.spreadplayers.usage", new Object[0]);
        }
        int n2 = 0;
        double d2 = br.a(ac2, Double.NaN, stringArray[n2++]);
        double d3 = br.a(ac2, Double.NaN, stringArray[n2++]);
        double d4 = br.a(ac2, stringArray[n2++], 0.0);
        double d5 = br.a(ac2, stringArray[n2++], d4 + 1.0);
        boolean bl2 = br.c(ac2, stringArray[n2++]);
        ArrayList<mw[]> arrayList = Lists.newArrayList();
        while (n2 < stringArray.length) {
            Object object;
            String string;
            if (ad.b(string = stringArray[n2++])) {
                object = ad.c(ac2, string);
                if (object != null && ((mw[])object).length != 0) {
                    Collections.addAll(arrayList, object);
                    continue;
                }
                throw new cg();
            }
            object = MinecraftServer.I().ah().a(string);
            if (object != null) {
                arrayList.add((mw[])object);
                continue;
            }
            throw new cg();
        }
        if (arrayList.isEmpty()) {
            throw new cg();
        }
        ac2.a(new fr("commands.spreadplayers.spreading." + (bl2 ? "teams" : "players"), arrayList.size(), d5, d2, d3, d4));
        this.a(ac2, arrayList, new bs(d2, d3), d4, d5, ((sv)arrayList.get((int)0)).o, bl2);
    }

    private void a(ac ac2, List list, bs bs2, double d2, double d3, ahb ahb2, boolean bl2) {
        Random random = new Random();
        double d4 = bs2.a - d3;
        double d5 = bs2.b - d3;
        double d6 = bs2.a + d3;
        double d7 = bs2.b + d3;
        bs[] bsArray = this.a(random, bl2 ? this.a(list) : list.size(), d4, d5, d6, d7);
        int n2 = this.a(bs2, d2, ahb2, random, d4, d5, d6, d7, bsArray, bl2);
        double d8 = this.a(list, ahb2, bsArray, bl2);
        br.a(ac2, (aa)this, "commands.spreadplayers.success." + (bl2 ? "teams" : "players"), bsArray.length, bs2.a, bs2.b);
        if (bsArray.length > 1) {
            ac2.a(new fr("commands.spreadplayers.info." + (bl2 ? "teams" : "players"), String.format("%.2f", d8), n2));
        }
    }

    private int a(List list) {
        HashSet<bae> hashSet = Sets.newHashSet();
        for (sv sv2 : list) {
            if (sv2 instanceof yz) {
                hashSet.add(sv2.bt());
                continue;
            }
            hashSet.add(null);
        }
        return hashSet.size();
    }

    private int a(bs bs2, double d2, ahb ahb2, Random random, double d3, double d4, double d5, double d6, bs[] bsArray, boolean bl2) {
        int n2;
        boolean bl3 = true;
        double d7 = 3.4028234663852886E38;
        for (n2 = 0; n2 < 10000 && bl3; ++n2) {
            bl3 = false;
            d7 = 3.4028234663852886E38;
            for (int i2 = 0; i2 < bsArray.length; ++i2) {
                bs bs3 = bsArray[i2];
                int n3 = 0;
                bs bs4 = new bs();
                for (int i3 = 0; i3 < bsArray.length; ++i3) {
                    if (i2 == i3) continue;
                    bs bs5 = bsArray[i3];
                    double d8 = bs3.a(bs5);
                    d7 = Math.min(d8, d7);
                    if (!(d8 < d2)) continue;
                    ++n3;
                    bs4.a += bs5.a - bs3.a;
                    bs4.b += bs5.b - bs3.b;
                }
                if (n3 > 0) {
                    bs4.a /= (double)n3;
                    bs4.b /= (double)n3;
                    double d9 = bs4.b();
                    if (d9 > 0.0) {
                        bs4.a();
                        bs3.b(bs4);
                    } else {
                        bs3.a(random, d3, d4, d5, d6);
                    }
                    bl3 = true;
                }
                if (!bs3.a(d3, d4, d5, d6)) continue;
                bl3 = true;
            }
            if (bl3) continue;
            for (bs bs4 : bsArray) {
                if (bs4.b(ahb2)) continue;
                bs4.a(random, d3, d4, d5, d6);
                bl3 = true;
            }
        }
        if (n2 >= 10000) {
            throw new cd("commands.spreadplayers.failure." + (bl2 ? "teams" : "players"), bsArray.length, bs2.a, bs2.b, String.format("%.2f", d7));
        }
        return n2;
    }

    private double a(List list, ahb ahb2, bs[] bsArray, boolean bl2) {
        double d2 = 0.0;
        int n2 = 0;
        HashMap<bae, bs> hashMap = Maps.newHashMap();
        for (int i2 = 0; i2 < list.size(); ++i2) {
            bs bs2;
            sv sv2 = (sv)list.get(i2);
            if (bl2) {
                bae bae2;
                bae bae3 = bae2 = sv2 instanceof yz ? sv2.bt() : null;
                if (!hashMap.containsKey(bae2)) {
                    hashMap.put(bae2, bsArray[n2++]);
                }
                bs2 = (bs)hashMap.get(bae2);
            } else {
                bs2 = bsArray[n2++];
            }
            sv2.a((double)((float)qh.c(bs2.a) + 0.5f), (double)bs2.a(ahb2), (double)qh.c(bs2.b) + 0.5);
            double d3 = Double.MAX_VALUE;
            for (int i3 = 0; i3 < bsArray.length; ++i3) {
                if (bs2 == bsArray[i3]) continue;
                double d4 = bs2.a(bsArray[i3]);
                d3 = Math.min(d4, d3);
            }
            d2 += d3;
        }
        return d2 /= (double)list.size();
    }

    private bs[] a(Random random, int n2, double d2, double d3, double d4, double d5) {
        bs[] bsArray = new bs[n2];
        for (int i2 = 0; i2 < bsArray.length; ++i2) {
            bs bs2 = new bs();
            bs2.a(random, d2, d3, d4, d5);
            bsArray[i2] = bs2;
        }
        return bsArray;
    }
}

