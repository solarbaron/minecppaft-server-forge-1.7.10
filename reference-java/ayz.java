/*
 * Decompiled with CFR 0.152.
 */
import java.util.concurrent.Callable;

class ayz
implements Callable {
    final /* synthetic */ ays a;

    ayz(ays ays2) {
        this.a = ays2;
    }

    public String a() {
        String string = "Unknown?";
        try {
            switch (ays.j(this.a)) {
                case 19133: {
                    string = "Anvil";
                    break;
                }
                case 19132: {
                    string = "McRegion";
                }
            }
        }
        catch (Throwable throwable) {
            // empty catch block
        }
        return String.format("0x%05X - %s", ays.j(this.a), string);
    }

    public /* synthetic */ Object call() {
        return this.a();
    }
}

