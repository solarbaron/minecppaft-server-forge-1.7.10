/*
 * Decompiled with CFR 0.152.
 */
import java.util.regex.Pattern;

public class qn {
    private static final Pattern a = Pattern.compile("(?i)\\u00A7[0-9A-FK-OR]");

    public static boolean b(String string) {
        return string == null || "".equals(string);
    }
}

