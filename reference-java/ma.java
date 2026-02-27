/*
 * Decompiled with CFR 0.152.
 */
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.swing.JTextField;
import net.minecraft.server.MinecraftServer;

class ma
implements ActionListener {
    final /* synthetic */ JTextField a;
    final /* synthetic */ ly b;

    ma(ly ly2, JTextField jTextField) {
        this.b = ly2;
        this.a = jTextField;
    }

    @Override
    public void actionPerformed(ActionEvent actionEvent) {
        String string = this.a.getText().trim();
        if (string.length() > 0) {
            ly.a(this.b).a(string, MinecraftServer.I());
        }
        this.a.setText("");
    }
}

