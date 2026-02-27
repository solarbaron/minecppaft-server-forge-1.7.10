/*
 * Decompiled with CFR 0.152.
 */
import java.util.Stack;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class eb {
    private static final Logger a = LogManager.getLogger();

    public static dy a(String string) {
        int n2 = eb.b(string = string.trim());
        if (n2 != 1) {
            throw new ea("Encountered multiple top tags, only one expected");
        }
        ec ec2 = null;
        ec2 = string.startsWith("{") ? eb.a("tag", string) : eb.a(eb.b(string, false), eb.c(string, false));
        return ec2.a();
    }

    static int b(String string) {
        int n2 = 0;
        boolean bl2 = false;
        Stack<Character> stack = new Stack<Character>();
        for (int i2 = 0; i2 < string.length(); ++i2) {
            char c2 = string.charAt(i2);
            if (c2 == '\"') {
                if (i2 > 0 && string.charAt(i2 - 1) == '\\') {
                    if (bl2) continue;
                    throw new ea("Illegal use of \\\": " + string);
                }
                bl2 = !bl2;
                continue;
            }
            if (bl2) continue;
            if (c2 == '{' || c2 == '[') {
                if (stack.isEmpty()) {
                    ++n2;
                }
                stack.push(Character.valueOf(c2));
                continue;
            }
            if (c2 == '}' && (stack.isEmpty() || ((Character)stack.pop()).charValue() != '{')) {
                throw new ea("Unbalanced curly brackets {}: " + string);
            }
            if (c2 != ']' || !stack.isEmpty() && ((Character)stack.pop()).charValue() == '[') continue;
            throw new ea("Unbalanced square brackets []: " + string);
        }
        if (bl2) {
            throw new ea("Unbalanced quotation: " + string);
        }
        if (!stack.isEmpty()) {
            throw new ea("Unbalanced brackets: " + string);
        }
        if (n2 == 0 && !string.isEmpty()) {
            return 1;
        }
        return n2;
    }

    static ec a(String string, String string2) {
        string2 = string2.trim();
        eb.b(string2);
        if (string2.startsWith("{")) {
            if (!string2.endsWith("}")) {
                throw new ea("Unable to locate ending bracket for: " + string2);
            }
            string2 = string2.substring(1, string2.length() - 1);
            ed ed2 = new ed(string);
            while (string2.length() > 0) {
                String string3 = eb.a(string2, false);
                if (string3.length() <= 0) continue;
                String string4 = eb.b(string3, false);
                String string5 = eb.c(string3, false);
                ed2.b.add(eb.a(string4, string5));
                if (string2.length() < string3.length() + 1) break;
                char c2 = string2.charAt(string3.length());
                if (c2 != ',' && c2 != '{' && c2 != '}' && c2 != '[' && c2 != ']') {
                    throw new ea("Unexpected token '" + c2 + "' at: " + string2.substring(string3.length()));
                }
                string2 = string2.substring(string3.length() + 1);
            }
            return ed2;
        }
        if (string2.startsWith("[") && !string2.matches("\\[[-\\d|,\\s]+\\]")) {
            if (!string2.endsWith("]")) {
                throw new ea("Unable to locate ending bracket for: " + string2);
            }
            string2 = string2.substring(1, string2.length() - 1);
            ee ee2 = new ee(string);
            while (string2.length() > 0) {
                String string6 = eb.a(string2, true);
                if (string6.length() > 0) {
                    String string7 = eb.b(string6, true);
                    String string8 = eb.c(string6, true);
                    ee2.b.add(eb.a(string7, string8));
                    if (string2.length() < string6.length() + 1) break;
                    char c3 = string2.charAt(string6.length());
                    if (c3 != ',' && c3 != '{' && c3 != '}' && c3 != '[' && c3 != ']') {
                        throw new ea("Unexpected token '" + c3 + "' at: " + string2.substring(string6.length()));
                    }
                    string2 = string2.substring(string6.length() + 1);
                    continue;
                }
                a.debug(string2);
            }
            return ee2;
        }
        return new ef(string, string2);
    }

    private static String a(String string, boolean bl2) {
        int n2;
        int n3 = eb.a(string, ':');
        if (n3 < 0 && !bl2) {
            throw new ea("Unable to locate name/value separator for string: " + string);
        }
        int n4 = eb.a(string, ',');
        if (n4 >= 0 && n4 < n3 && !bl2) {
            throw new ea("Name error at: " + string);
        }
        if (bl2 && (n3 < 0 || n3 > n4)) {
            n3 = -1;
        }
        Stack<Character> stack = new Stack<Character>();
        boolean bl3 = false;
        boolean bl4 = false;
        boolean bl5 = false;
        int n5 = 0;
        for (n2 = n3 + 1; n2 < string.length(); ++n2) {
            char c2 = string.charAt(n2);
            if (c2 == '\"') {
                if (n2 > 0 && string.charAt(n2 - 1) == '\\') {
                    if (!bl3) {
                        throw new ea("Illegal use of \\\": " + string);
                    }
                } else {
                    boolean bl6 = bl3 = !bl3;
                    if (bl3 && !bl5) {
                        bl4 = true;
                    }
                    if (!bl3) {
                        n5 = n2;
                    }
                }
            } else if (!bl3) {
                if (c2 == '{' || c2 == '[') {
                    stack.push(Character.valueOf(c2));
                } else {
                    if (c2 == '}' && (stack.isEmpty() || ((Character)stack.pop()).charValue() != '{')) {
                        throw new ea("Unbalanced curly brackets {}: " + string);
                    }
                    if (c2 == ']' && (stack.isEmpty() || ((Character)stack.pop()).charValue() != '[')) {
                        throw new ea("Unbalanced square brackets []: " + string);
                    }
                    if (c2 == ',' && stack.isEmpty()) {
                        return string.substring(0, n2);
                    }
                }
            }
            if (Character.isWhitespace(c2)) continue;
            if (!bl3 && bl4 && n5 != n2) {
                return string.substring(0, n5 + 1);
            }
            bl5 = true;
        }
        return string.substring(0, n2);
    }

    private static String b(String string, boolean bl2) {
        if (bl2 && ((string = string.trim()).startsWith("{") || string.startsWith("["))) {
            return "";
        }
        int n2 = string.indexOf(58);
        if (n2 < 0) {
            if (bl2) {
                return "";
            }
            throw new ea("Unable to locate name/value separator for string: " + string);
        }
        return string.substring(0, n2).trim();
    }

    private static String c(String string, boolean bl2) {
        if (bl2 && ((string = string.trim()).startsWith("{") || string.startsWith("["))) {
            return string;
        }
        int n2 = string.indexOf(58);
        if (n2 < 0) {
            if (bl2) {
                return string;
            }
            throw new ea("Unable to locate name/value separator for string: " + string);
        }
        return string.substring(n2 + 1).trim();
    }

    private static int a(String string, char c2) {
        boolean bl2 = false;
        for (int i2 = 0; i2 < string.length(); ++i2) {
            char c3 = string.charAt(i2);
            if (c3 == '\"') {
                if (i2 > 0 && string.charAt(i2 - 1) == '\\') continue;
                bl2 = !bl2;
                continue;
            }
            if (bl2) continue;
            if (c3 == c2) {
                return i2;
            }
            if (c3 != '{' && c3 != '[') continue;
            return -1;
        }
        return -1;
    }
}

