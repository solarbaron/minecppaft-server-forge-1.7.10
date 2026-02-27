/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;

class ee
extends ec {
    protected ArrayList b = new ArrayList();

    public ee(String string) {
        this.a = string;
    }

    @Override
    public dy a() {
        dq dq2 = new dq();
        for (ec ec2 : this.b) {
            dq2.a(ec2.a());
        }
        return dq2;
    }
}

