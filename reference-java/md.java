/*
 * Decompiled with CFR 0.152.
 */
import javax.swing.JScrollPane;
import javax.swing.JTextArea;

class md
implements Runnable {
    final /* synthetic */ JTextArea a;
    final /* synthetic */ JScrollPane b;
    final /* synthetic */ String c;
    final /* synthetic */ ly d;

    md(ly ly2, JTextArea jTextArea, JScrollPane jScrollPane, String string) {
        this.d = ly2;
        this.a = jTextArea;
        this.b = jScrollPane;
        this.c = string;
    }

    @Override
    public void run() {
        this.d.a(this.a, this.b, this.c);
    }
}

