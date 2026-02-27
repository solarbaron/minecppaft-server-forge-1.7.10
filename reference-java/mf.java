/*
 * Decompiled with CFR 0.152.
 */
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.text.DecimalFormat;
import javax.swing.JComponent;
import javax.swing.Timer;
import net.minecraft.server.MinecraftServer;

public class mf
extends JComponent {
    private static final DecimalFormat a = new DecimalFormat("########0.000");
    private int[] b = new int[256];
    private int c;
    private String[] d = new String[11];
    private final MinecraftServer e;

    public mf(MinecraftServer minecraftServer) {
        this.e = minecraftServer;
        this.setPreferredSize(new Dimension(456, 246));
        this.setMinimumSize(new Dimension(456, 246));
        this.setMaximumSize(new Dimension(456, 246));
        new Timer(500, new mg(this)).start();
        this.setBackground(Color.BLACK);
    }

    private void a() {
        long l2 = Runtime.getRuntime().totalMemory() - Runtime.getRuntime().freeMemory();
        System.gc();
        this.d[0] = "Memory use: " + l2 / 1024L / 1024L + " mb (" + Runtime.getRuntime().freeMemory() * 100L / Runtime.getRuntime().maxMemory() + "% free)";
        this.d[1] = "Avg tick: " + a.format(this.a(this.e.g) * 1.0E-6) + " ms";
        this.repaint();
    }

    private double a(long[] lArray) {
        long l2 = 0L;
        for (int i2 = 0; i2 < lArray.length; ++i2) {
            l2 += lArray[i2];
        }
        return (double)l2 / (double)lArray.length;
    }

    @Override
    public void paint(Graphics graphics) {
        int n2;
        graphics.setColor(new Color(0xFFFFFF));
        graphics.fillRect(0, 0, 456, 246);
        for (n2 = 0; n2 < 256; ++n2) {
            int n3 = this.b[n2 + this.c & 0xFF];
            graphics.setColor(new Color(n3 + 28 << 16));
            graphics.fillRect(n2, 100 - n3, 1, n3);
        }
        graphics.setColor(Color.BLACK);
        for (n2 = 0; n2 < this.d.length; ++n2) {
            String string = this.d[n2];
            if (string == null) continue;
            graphics.drawString(string, 32, 116 + n2 * 16);
        }
    }

    static /* synthetic */ void a(mf mf2) {
        mf2.a();
    }
}

