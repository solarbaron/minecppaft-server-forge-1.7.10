/*
 * Decompiled with CFR 0.152.
 */
import java.io.File;
import java.io.FileWriter;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import org.apache.commons.io.IOUtils;
import org.apache.commons.lang3.ArrayUtils;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class b {
    private static final Logger a = LogManager.getLogger();
    private final String b;
    private final Throwable c;
    private final k d = new k(this, "System Details");
    private final List e = new ArrayList();
    private File f;
    private boolean g = true;
    private StackTraceElement[] h = new StackTraceElement[0];

    public b(String string, Throwable throwable) {
        this.b = string;
        this.c = throwable;
        this.h();
    }

    private void h() {
        this.d.a("Minecraft Version", new c(this));
        this.d.a("Operating System", new d(this));
        this.d.a("Java Version", new e(this));
        this.d.a("Java VM Version", new f(this));
        this.d.a("Memory", new g(this));
        this.d.a("JVM Flags", new h(this));
        this.d.a("AABB Pool Size", new i(this));
        this.d.a("IntCache", new j(this));
    }

    public String a() {
        return this.b;
    }

    public Throwable b() {
        return this.c;
    }

    public void a(StringBuilder stringBuilder) {
        if ((this.h == null || this.h.length <= 0) && this.e.size() > 0) {
            this.h = ArrayUtils.subarray(((k)this.e.get(0)).a(), 0, 1);
        }
        if (this.h != null && this.h.length > 0) {
            stringBuilder.append("-- Head --\n");
            stringBuilder.append("Stacktrace:\n");
            for (StackTraceElement stackTraceElement : this.h) {
                stringBuilder.append("\t").append("at ").append(stackTraceElement.toString());
                stringBuilder.append("\n");
            }
            stringBuilder.append("\n");
        }
        for (k k2 : this.e) {
            k2.a(stringBuilder);
            stringBuilder.append("\n\n");
        }
        this.d.a(stringBuilder);
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    public String d() {
        StringWriter stringWriter = null;
        PrintWriter printWriter = null;
        Throwable throwable = this.c;
        if (throwable.getMessage() == null) {
            if (throwable instanceof NullPointerException) {
                throwable = new NullPointerException(this.b);
            } else if (throwable instanceof StackOverflowError) {
                throwable = new StackOverflowError(this.b);
            } else if (throwable instanceof OutOfMemoryError) {
                throwable = new OutOfMemoryError(this.b);
            }
            throwable.setStackTrace(this.c.getStackTrace());
        }
        String string = throwable.toString();
        try {
            stringWriter = new StringWriter();
            printWriter = new PrintWriter(stringWriter);
            throwable.printStackTrace(printWriter);
            string = stringWriter.toString();
        }
        catch (Throwable throwable2) {
            IOUtils.closeQuietly(stringWriter);
            IOUtils.closeQuietly(printWriter);
            throw throwable2;
        }
        IOUtils.closeQuietly(stringWriter);
        IOUtils.closeQuietly(printWriter);
        return string;
    }

    public String e() {
        StringBuilder stringBuilder = new StringBuilder();
        stringBuilder.append("---- Minecraft Crash Report ----\n");
        stringBuilder.append("// ");
        stringBuilder.append(b.i());
        stringBuilder.append("\n\n");
        stringBuilder.append("Time: ");
        stringBuilder.append(new SimpleDateFormat().format(new Date()));
        stringBuilder.append("\n");
        stringBuilder.append("Description: ");
        stringBuilder.append(this.b);
        stringBuilder.append("\n\n");
        stringBuilder.append(this.d());
        stringBuilder.append("\n\nA detailed walkthrough of the error, its code path and all known details is as follows:\n");
        for (int i2 = 0; i2 < 87; ++i2) {
            stringBuilder.append("-");
        }
        stringBuilder.append("\n\n");
        this.a(stringBuilder);
        return stringBuilder.toString();
    }

    public boolean a(File file) {
        if (this.f != null) {
            return false;
        }
        if (file.getParentFile() != null) {
            file.getParentFile().mkdirs();
        }
        try {
            FileWriter fileWriter = new FileWriter(file);
            fileWriter.write(this.e());
            fileWriter.close();
            this.f = file;
            return true;
        }
        catch (Throwable throwable) {
            a.error("Could not save crash report to " + file, throwable);
            return false;
        }
    }

    public k g() {
        return this.d;
    }

    public k a(String string) {
        return this.a(string, 1);
    }

    public k a(String string, int n2) {
        k k2 = new k(this, string);
        if (this.g) {
            int n3 = k2.a(n2);
            StackTraceElement[] stackTraceElementArray = this.c.getStackTrace();
            StackTraceElement stackTraceElement = null;
            StackTraceElement stackTraceElement2 = null;
            int n4 = stackTraceElementArray.length - n3;
            if (n4 < 0) {
                System.out.println("Negative index in crash report handler (" + stackTraceElementArray.length + "/" + n3 + ")");
            }
            if (stackTraceElementArray != null && 0 <= n4 && n4 < stackTraceElementArray.length) {
                stackTraceElement = stackTraceElementArray[n4];
                if (stackTraceElementArray.length + 1 - n3 < stackTraceElementArray.length) {
                    stackTraceElement2 = stackTraceElementArray[stackTraceElementArray.length + 1 - n3];
                }
            }
            this.g = k2.a(stackTraceElement, stackTraceElement2);
            if (n3 > 0 && !this.e.isEmpty()) {
                k k3 = (k)this.e.get(this.e.size() - 1);
                k3.b(n3);
            } else if (stackTraceElementArray != null && stackTraceElementArray.length >= n3 && 0 <= n4 && n4 < stackTraceElementArray.length) {
                this.h = new StackTraceElement[n4];
                System.arraycopy(stackTraceElementArray, 0, this.h, 0, this.h.length);
            } else {
                this.g = false;
            }
        }
        this.e.add(k2);
        return k2;
    }

    private static String i() {
        String[] stringArray = new String[]{"Who set us up the TNT?", "Everything's going to plan. No, really, that was supposed to happen.", "Uh... Did I do that?", "Oops.", "Why did you do that?", "I feel sad now :(", "My bad.", "I'm sorry, Dave.", "I let you down. Sorry :(", "On the bright side, I bought you a teddy bear!", "Daisy, daisy...", "Oh - I know what I did wrong!", "Hey, that tickles! Hehehe!", "I blame Dinnerbone.", "You should try our sister game, Minceraft!", "Don't be sad. I'll do better next time, I promise!", "Don't be sad, have a hug! <3", "I just don't know what went wrong :(", "Shall we play a game?", "Quite honestly, I wouldn't worry myself about that.", "I bet Cylons wouldn't have this problem.", "Sorry :(", "Surprise! Haha. Well, this is awkward.", "Would you like a cupcake?", "Hi. I'm Minecraft, and I'm a crashaholic.", "Ooh. Shiny.", "This doesn't make any sense!", "Why is it breaking :(", "Don't do that.", "Ouch. That hurt :(", "You're mean.", "This is a token for 1 free hug. Redeem at your nearest Mojangsta: [~~HUG~~]", "There are four lights!", "But it works on my machine."};
        try {
            return stringArray[(int)(System.nanoTime() % (long)stringArray.length)];
        }
        catch (Throwable throwable) {
            return "Witty comment unavailable :(";
        }
    }

    public static b a(Throwable throwable, String string) {
        b b2 = throwable instanceof s ? ((s)throwable).a() : new b(string, throwable);
        return b2;
    }
}

