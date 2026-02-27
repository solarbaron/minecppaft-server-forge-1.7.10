/*
 * Decompiled with CFR 0.152.
 */
import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.Font;
import javax.swing.JComponent;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JScrollBar;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.SwingUtilities;
import javax.swing.UIManager;
import javax.swing.border.EtchedBorder;
import javax.swing.border.TitledBorder;
import javax.swing.text.BadLocationException;
import javax.swing.text.Document;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class ly
extends JComponent {
    private static final Font a = new Font("Monospaced", 0, 12);
    private static final Logger b = LogManager.getLogger();
    private lt c;

    public static void a(lt lt2) {
        try {
            UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
        }
        catch (Exception exception) {
            // empty catch block
        }
        ly ly2 = new ly(lt2);
        JFrame jFrame = new JFrame("Minecraft server");
        jFrame.add(ly2);
        jFrame.pack();
        jFrame.setLocationRelativeTo(null);
        jFrame.setVisible(true);
        jFrame.addWindowListener(new lz(lt2));
    }

    public ly(lt lt2) {
        this.c = lt2;
        this.setPreferredSize(new Dimension(854, 480));
        this.setLayout(new BorderLayout());
        try {
            this.add((Component)this.c(), "Center");
            this.add((Component)this.a(), "West");
        }
        catch (Exception exception) {
            b.error("Couldn't build server GUI", (Throwable)exception);
        }
    }

    private JComponent a() {
        JPanel jPanel = new JPanel(new BorderLayout());
        jPanel.add((Component)new mf(this.c), "North");
        jPanel.add((Component)this.b(), "Center");
        jPanel.setBorder(new TitledBorder(new EtchedBorder(), "Stats"));
        return jPanel;
    }

    private JComponent b() {
        me me2 = new me(this.c);
        JScrollPane jScrollPane = new JScrollPane(me2, 22, 30);
        jScrollPane.setBorder(new TitledBorder(new EtchedBorder(), "Players"));
        return jScrollPane;
    }

    private JComponent c() {
        JPanel jPanel = new JPanel(new BorderLayout());
        JTextArea jTextArea = new JTextArea();
        JScrollPane jScrollPane = new JScrollPane(jTextArea, 22, 30);
        jTextArea.setEditable(false);
        jTextArea.setFont(a);
        JTextField jTextField = new JTextField();
        jTextField.addActionListener(new ma(this, jTextField));
        jTextArea.addFocusListener(new mb(this));
        jPanel.add((Component)jScrollPane, "Center");
        jPanel.add((Component)jTextField, "South");
        jPanel.setBorder(new TitledBorder(new EtchedBorder(), "Log and chat"));
        Thread thread = new Thread(new mc(this, jTextArea, jScrollPane));
        thread.setDaemon(true);
        thread.start();
        return jPanel;
    }

    public void a(JTextArea jTextArea, JScrollPane jScrollPane, String string) {
        if (!SwingUtilities.isEventDispatchThread()) {
            SwingUtilities.invokeLater(new md(this, jTextArea, jScrollPane, string));
            return;
        }
        Document document = jTextArea.getDocument();
        JScrollBar jScrollBar = jScrollPane.getVerticalScrollBar();
        boolean bl2 = false;
        if (jScrollPane.getViewport().getView() == jTextArea) {
            bl2 = (double)jScrollBar.getValue() + jScrollBar.getSize().getHeight() + (double)(a.getSize() * 4) > (double)jScrollBar.getMaximum();
        }
        try {
            document.insertString(document.getLength(), string, null);
        }
        catch (BadLocationException badLocationException) {
            // empty catch block
        }
        if (bl2) {
            jScrollBar.setValue(Integer.MAX_VALUE);
        }
    }

    static /* synthetic */ lt a(ly ly2) {
        return ly2.c;
    }
}

