/*
 * Decompiled with CFR 0.152.
 */
public class apv {
    public final byte[] a;
    private final int b;
    private final int c;

    public apv(int n2, int n3) {
        this.a = new byte[n2 >> 1];
        this.b = n3;
        this.c = n3 + 4;
    }

    public apv(byte[] byArray, int n2) {
        this.a = byArray;
        this.b = n2;
        this.c = n2 + 4;
    }

    public int a(int n2, int n3, int n4) {
        int n5 = n3 << this.c | n4 << this.b | n2;
        int n6 = n5 >> 1;
        int n7 = n5 & 1;
        if (n7 == 0) {
            return this.a[n6] & 0xF;
        }
        return this.a[n6] >> 4 & 0xF;
    }

    public void a(int n2, int n3, int n4, int n5) {
        int n6 = n3 << this.c | n4 << this.b | n2;
        int n7 = n6 >> 1;
        int n8 = n6 & 1;
        this.a[n7] = n8 == 0 ? (byte)(this.a[n7] & 0xF0 | n5 & 0xF) : (byte)(this.a[n7] & 0xF | (n5 & 0xF) << 4);
    }
}

