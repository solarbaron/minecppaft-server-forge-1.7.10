/*
 * Decompiled with CFR 0.152.
 */
import java.util.Arrays;
import java.util.List;
import java.util.Random;

public class aie
extends aib {
    private ahu c;
    private float d;

    public aie(ahu ahu2, float f2) {
        this.c = ahu2;
        this.d = f2;
    }

    @Override
    public ahu a(int n2, int n3) {
        return this.c;
    }

    @Override
    public ahu[] a(ahu[] ahuArray, int n2, int n3, int n4, int n5) {
        if (ahuArray == null || ahuArray.length < n4 * n5) {
            ahuArray = new ahu[n4 * n5];
        }
        Arrays.fill(ahuArray, 0, n4 * n5, this.c);
        return ahuArray;
    }

    @Override
    public float[] a(float[] fArray, int n2, int n3, int n4, int n5) {
        if (fArray == null || fArray.length < n4 * n5) {
            fArray = new float[n4 * n5];
        }
        Arrays.fill(fArray, 0, n4 * n5, this.d);
        return fArray;
    }

    @Override
    public ahu[] b(ahu[] ahuArray, int n2, int n3, int n4, int n5) {
        if (ahuArray == null || ahuArray.length < n4 * n5) {
            ahuArray = new ahu[n4 * n5];
        }
        Arrays.fill(ahuArray, 0, n4 * n5, this.c);
        return ahuArray;
    }

    @Override
    public ahu[] a(ahu[] ahuArray, int n2, int n3, int n4, int n5, boolean bl2) {
        return this.b(ahuArray, n2, n3, n4, n5);
    }

    @Override
    public agt a(int n2, int n3, int n4, List list, Random random) {
        if (list.contains(this.c)) {
            return new agt(n2 - n4 + random.nextInt(n4 * 2 + 1), 0, n3 - n4 + random.nextInt(n4 * 2 + 1));
        }
        return null;
    }

    @Override
    public boolean a(int n2, int n3, int n4, List list) {
        return list.contains(this.c);
    }
}

