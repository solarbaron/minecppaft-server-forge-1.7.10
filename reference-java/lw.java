/*
 * Decompiled with CFR 0.152.
 */
import java.util.concurrent.Callable;

class lw
implements Callable {
    final /* synthetic */ lt a;

    lw(lt lt2) {
        this.a = lt2;
    }

    public String a() {
        String string = this.a.getServerModName();
        if (!string.equals("vanilla")) {
            return "Definitely; Server brand changed to '" + string + "'";
        }
        return "Unknown (can't tell)";
    }

    public /* synthetic */ Object call() {
        return this.a();
    }
}

