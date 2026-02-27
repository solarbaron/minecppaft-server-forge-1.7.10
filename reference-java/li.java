/*
 * Decompiled with CFR 0.152.
 */
public final class li
extends Thread {
    final /* synthetic */ lt a;

    public li(String string, lt lt2) {
        this.a = lt2;
        super(string);
    }

    @Override
    public void run() {
        this.a.o();
    }
}

