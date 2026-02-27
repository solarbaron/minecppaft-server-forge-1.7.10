/*
 * Decompiled with CFR 0.152.
 */
import com.google.common.collect.Sets;
import java.util.Set;

public class adn
extends acg {
    private static final Set c = Sets.newHashSet(ajn.e, ajn.T, ajn.U, ajn.b, ajn.A, ajn.Y, ajn.p, ajn.S, ajn.q, ajn.R, ajn.o, ajn.ag, ajn.ah, ajn.aD, ajn.aL, ajn.x, ajn.y, ajn.ax, ajn.ay, ajn.aq, ajn.E, ajn.D, ajn.cc);

    protected adn(adc adc2) {
        super(2.0f, adc2, c);
    }

    @Override
    public boolean b(aji aji2) {
        if (aji2 == ajn.Z) {
            return this.b.d() == 3;
        }
        if (aji2 == ajn.ah || aji2 == ajn.ag) {
            return this.b.d() >= 2;
        }
        if (aji2 == ajn.bA || aji2 == ajn.bE) {
            return this.b.d() >= 2;
        }
        if (aji2 == ajn.R || aji2 == ajn.o) {
            return this.b.d() >= 2;
        }
        if (aji2 == ajn.S || aji2 == ajn.p) {
            return this.b.d() >= 1;
        }
        if (aji2 == ajn.y || aji2 == ajn.x) {
            return this.b.d() >= 1;
        }
        if (aji2 == ajn.ax || aji2 == ajn.ay) {
            return this.b.d() >= 2;
        }
        if (aji2.o() == awt.e) {
            return true;
        }
        if (aji2.o() == awt.f) {
            return true;
        }
        return aji2.o() == awt.g;
    }

    @Override
    public float a(add add2, aji aji2) {
        if (aji2.o() == awt.f || aji2.o() == awt.g || aji2.o() == awt.e) {
            return this.a;
        }
        return super.a(add2, aji2);
    }
}

