/*
 * Decompiled with CFR 0.152.
 */
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;

final class lz
extends WindowAdapter {
    final /* synthetic */ lt a;

    lz(lt lt2) {
        this.a = lt2;
    }

    @Override
    public void windowClosing(WindowEvent windowEvent) {
        this.a.r();
        while (!this.a.ag()) {
            try {
                Thread.sleep(100L);
            }
            catch (InterruptedException interruptedException) {
                interruptedException.printStackTrace();
            }
        }
        System.exit(0);
    }
}

