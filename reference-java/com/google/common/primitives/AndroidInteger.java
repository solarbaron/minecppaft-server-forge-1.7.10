/*
 * Decompiled with CFR 0.152.
 */
package com.google.common.primitives;

import com.google.common.base.Preconditions;
import javax.annotation.CheckForNull;

final class AndroidInteger {
    @CheckForNull
    static Integer tryParse(String string) {
        return AndroidInteger.tryParse(string, 10);
    }

    @CheckForNull
    static Integer tryParse(String string, int radix) {
        boolean negative;
        Preconditions.checkNotNull(string);
        Preconditions.checkArgument(radix >= 2, "Invalid radix %s, min radix is %s", radix, 2);
        Preconditions.checkArgument(radix <= 36, "Invalid radix %s, max radix is %s", radix, 36);
        int length = string.length();
        int i2 = 0;
        if (length == 0) {
            return null;
        }
        boolean bl2 = negative = string.charAt(i2) == '-';
        if (negative && ++i2 == length) {
            return null;
        }
        return AndroidInteger.tryParse(string, i2, radix, negative);
    }

    @CheckForNull
    private static Integer tryParse(String string, int offset, int radix, boolean negative) {
        int max = Integer.MIN_VALUE / radix;
        int result = 0;
        int length = string.length();
        while (offset < length) {
            int digit;
            if ((digit = Character.digit(string.charAt(offset++), radix)) == -1) {
                return null;
            }
            if (max > result) {
                return null;
            }
            int next = result * radix - digit;
            if (next > result) {
                return null;
            }
            result = next;
        }
        if (!negative && (result = -result) < 0) {
            return null;
        }
        if (result > Integer.MAX_VALUE || result < Integer.MIN_VALUE) {
            return null;
        }
        return result;
    }

    private AndroidInteger() {
    }
}

