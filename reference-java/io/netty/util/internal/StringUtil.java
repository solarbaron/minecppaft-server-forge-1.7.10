/*
 * Decompiled with CFR 0.152.
 */
package io.netty.util.internal;

import java.util.ArrayList;
import java.util.Formatter;

public final class StringUtil {
    public static final String NEWLINE;
    private static final String EMPTY_STRING = "";

    private StringUtil() {
    }

    public static String[] split(String value, char delim) {
        int i2;
        int end = value.length();
        ArrayList<String> res = new ArrayList<String>();
        int start = 0;
        for (i2 = 0; i2 < end; ++i2) {
            if (value.charAt(i2) != delim) continue;
            if (start == i2) {
                res.add(EMPTY_STRING);
            } else {
                res.add(value.substring(start, i2));
            }
            start = i2 + 1;
        }
        if (start == 0) {
            res.add(value);
        } else if (start != end) {
            res.add(value.substring(start, end));
        } else {
            for (i2 = res.size() - 1; i2 >= 0 && ((String)res.get(i2)).isEmpty(); --i2) {
                res.remove(i2);
            }
        }
        return res.toArray(new String[res.size()]);
    }

    public static String simpleClassName(Object o2) {
        return StringUtil.simpleClassName(o2.getClass());
    }

    public static String simpleClassName(Class<?> clazz) {
        Package pkg = clazz.getPackage();
        if (pkg != null) {
            return clazz.getName().substring(pkg.getName().length() + 1);
        }
        return clazz.getName();
    }

    static {
        String newLine;
        try {
            newLine = new Formatter().format("%n", new Object[0]).toString();
        }
        catch (Exception e2) {
            newLine = "\n";
        }
        NEWLINE = newLine;
    }
}

