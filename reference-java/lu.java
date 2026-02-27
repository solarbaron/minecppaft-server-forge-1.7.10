/*
 * Decompiled with CFR 0.152.
 */
class lu
extends Thread {
    final /* synthetic */ lt a;

    lu(lt lt2, String string) {
        this.a = lt2;
        super(string);
        this.setDaemon(true);
        this.start();
    }

    @Override
    public void run() {
        while (true) {
            try {
                while (true) {
                    Thread.sleep(Integer.MAX_VALUE);
                }
            }
            catch (InterruptedException interruptedException) {
                continue;
            }
            break;
        }
    }
}

