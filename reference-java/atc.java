/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class atc
extends avm {
    public atc() {
    }

    public atc(ahb ahb2, Random random, int n2, int n3) {
        super(n2, n3);
        ata ata2 = new ata(0, random, (n2 << 4) + 2, (n3 << 4) + 2);
        this.a.add(ata2);
        ata2.a(ata2, this.a, random);
        this.c();
        this.a(ahb2, random, 10);
    }
}

