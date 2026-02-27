/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Callable;

public class k {
    private final b a;
    private final String b;
    private final List c = new ArrayList();
    private StackTraceElement[] d = new StackTraceElement[0];

    public k(b b2, String string) {
        this.a = b2;
        this.b = string;
    }

    public static String a(int n2, int n3, int n4) {
        int n5;
        int n6;
        int n7;
        int n8;
        int n9;
        int n10;
        int n11;
        int n12;
        StringBuilder stringBuilder = new StringBuilder();
        try {
            stringBuilder.append(String.format("World: (%d,%d,%d)", n2, n3, n4));
        }
        catch (Throwable throwable) {
            stringBuilder.append("(Error finding world loc)");
        }
        stringBuilder.append(", ");
        try {
            int n13 = n2 >> 4;
            n12 = n4 >> 4;
            n11 = n2 & 0xF;
            n10 = n3 >> 4;
            n9 = n4 & 0xF;
            n8 = n13 << 4;
            n7 = n12 << 4;
            n6 = (n13 + 1 << 4) - 1;
            n5 = (n12 + 1 << 4) - 1;
            stringBuilder.append(String.format("Chunk: (at %d,%d,%d in %d,%d; contains blocks %d,0,%d to %d,255,%d)", n11, n10, n9, n13, n12, n8, n7, n6, n5));
        }
        catch (Throwable throwable) {
            stringBuilder.append("(Error finding chunk loc)");
        }
        stringBuilder.append(", ");
        try {
            int n14 = n2 >> 9;
            n12 = n4 >> 9;
            n11 = n14 << 5;
            n10 = n12 << 5;
            n9 = (n14 + 1 << 5) - 1;
            n8 = (n12 + 1 << 5) - 1;
            n7 = n14 << 9;
            n6 = n12 << 9;
            n5 = (n14 + 1 << 9) - 1;
            int n15 = (n12 + 1 << 9) - 1;
            stringBuilder.append(String.format("Region: (%d,%d; contains chunks %d,%d to %d,%d, blocks %d,0,%d to %d,255,%d)", n14, n12, n11, n10, n9, n8, n7, n6, n5, n15));
        }
        catch (Throwable throwable) {
            stringBuilder.append("(Error finding world loc)");
        }
        return stringBuilder.toString();
    }

    public void a(String string, Callable callable) {
        try {
            this.a(string, callable.call());
        }
        catch (Throwable throwable) {
            this.a(string, throwable);
        }
    }

    public void a(String string, Object object) {
        this.c.add(new o(string, object));
    }

    public void a(String string, Throwable throwable) {
        this.a(string, (Object)throwable);
    }

    public int a(int n2) {
        StackTraceElement[] stackTraceElementArray = Thread.currentThread().getStackTrace();
        if (stackTraceElementArray.length <= 0) {
            return 0;
        }
        this.d = new StackTraceElement[stackTraceElementArray.length - 3 - n2];
        System.arraycopy(stackTraceElementArray, 3 + n2, this.d, 0, this.d.length);
        return this.d.length;
    }

    public boolean a(StackTraceElement stackTraceElement, StackTraceElement stackTraceElement2) {
        if (this.d.length == 0 || stackTraceElement == null) {
            return false;
        }
        StackTraceElement stackTraceElement3 = this.d[0];
        if (!(stackTraceElement3.isNativeMethod() == stackTraceElement.isNativeMethod() && stackTraceElement3.getClassName().equals(stackTraceElement.getClassName()) && stackTraceElement3.getFileName().equals(stackTraceElement.getFileName()) && stackTraceElement3.getMethodName().equals(stackTraceElement.getMethodName()))) {
            return false;
        }
        if (stackTraceElement2 != null != this.d.length > 1) {
            return false;
        }
        if (stackTraceElement2 != null && !this.d[1].equals(stackTraceElement2)) {
            return false;
        }
        this.d[0] = stackTraceElement;
        return true;
    }

    public void b(int n2) {
        StackTraceElement[] stackTraceElementArray = new StackTraceElement[this.d.length - n2];
        System.arraycopy(this.d, 0, stackTraceElementArray, 0, stackTraceElementArray.length);
        this.d = stackTraceElementArray;
    }

    public void a(StringBuilder stringBuilder) {
        stringBuilder.append("-- ").append(this.b).append(" --\n");
        stringBuilder.append("Details:");
        for (o o2 : this.c) {
            stringBuilder.append("\n\t");
            stringBuilder.append(o2.a());
            stringBuilder.append(": ");
            stringBuilder.append(o2.b());
        }
        if (this.d != null && this.d.length > 0) {
            stringBuilder.append("\nStacktrace:");
            for (StackTraceElement stackTraceElement : this.d) {
                stringBuilder.append("\n\tat ");
                stringBuilder.append(stackTraceElement.toString());
            }
        }
    }

    public StackTraceElement[] a() {
        return this.d;
    }

    public static void a(k k2, int n2, int n3, int n4, aji aji2, int n5) {
        int n6 = aji.b(aji2);
        k2.a("Block type", new l(n6, aji2));
        k2.a("Block data value", new m(n5));
        k2.a("Block location", new n(n2, n3, n4));
    }
}

