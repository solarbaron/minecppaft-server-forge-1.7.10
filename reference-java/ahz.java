/*
 * Decompiled with CFR 0.152.
 */
public class ahz {
    public float[] a = new float[256];
    public ahu[] b = new ahu[256];
    public int c;
    public int d;
    public long e;
    final /* synthetic */ ahy f;

    public ahz(ahy ahy2, int n2, int n3) {
        this.f = ahy2;
        this.c = n2;
        this.d = n3;
        ahy.a(ahy2).a(this.a, n2 << 4, n3 << 4, 16, 16);
        ahy.a(ahy2).a(this.b, n2 << 4, n3 << 4, 16, 16, false);
    }

    public ahu a(int n2, int n3) {
        return this.b[n2 & 0xF | (n3 & 0xF) << 4];
    }
}

