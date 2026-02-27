/*
 * Decompiled with CFR 0.152.
 */
import java.util.Vector;
import javax.swing.JList;
import net.minecraft.server.MinecraftServer;

public class me
extends JList
implements lr {
    private MinecraftServer a;
    private int b;

    public me(MinecraftServer minecraftServer) {
        this.a = minecraftServer;
        minecraftServer.a(this);
    }

    @Override
    public void a() {
        if (this.b++ % 20 == 0) {
            Vector<String> vector = new Vector<String>();
            for (int i2 = 0; i2 < this.a.ah().e.size(); ++i2) {
                vector.add(((mw)this.a.ah().e.get(i2)).b_());
            }
            this.setListData(vector);
        }
    }
}

