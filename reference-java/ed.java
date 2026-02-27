/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;

class ed
extends ec {
    protected ArrayList b = new ArrayList();

    public ed(String string) {
        this.a = string;
    }

    @Override
    public dy a() {
        dh dh2 = new dh();
        for (ec ec2 : this.b) {
            dh2.a(ec2.a, ec2.a());
        }
        return dh2;
    }
}

