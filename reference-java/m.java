/*
 * Decompiled with CFR 0.152.
 */
import java.util.concurrent.Callable;

final class m
implements Callable {
    final /* synthetic */ int a;

    m(int n2) {
        this.a = n2;
    }

    public String a() {
        if (this.a < 0) {
            return "Unknown? (Got " + this.a + ")";
        }
        String string = String.format("%4s", Integer.toBinaryString(this.a)).replace(" ", "0");
        return String.format("%1$d / 0x%1$X / 0b%2$s", this.a, string);
    }

    public /* synthetic */ Object call() {
        return this.a();
    }
}

