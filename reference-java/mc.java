/*
 * Decompiled with CFR 0.152.
 */
import com.mojang.util.QueueLogAppender;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;

class mc
implements Runnable {
    final /* synthetic */ JTextArea a;
    final /* synthetic */ JScrollPane b;
    final /* synthetic */ ly c;

    mc(ly ly2, JTextArea jTextArea, JScrollPane jScrollPane) {
        this.c = ly2;
        this.a = jTextArea;
        this.b = jScrollPane;
    }

    @Override
    public void run() {
        String string;
        while ((string = QueueLogAppender.getNextLogEvent("ServerGuiConsole")) != null) {
            this.c.a(this.a, this.b, string);
        }
    }
}

