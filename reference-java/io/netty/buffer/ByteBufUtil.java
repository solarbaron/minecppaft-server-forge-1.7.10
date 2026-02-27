/*
 * Decompiled with CFR 0.152.
 */
package io.netty.buffer;

import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufAllocator;
import io.netty.util.CharsetUtil;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.CharBuffer;
import java.nio.charset.CharacterCodingException;
import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharsetEncoder;
import java.nio.charset.CoderResult;

public final class ByteBufUtil {
    private static final char[] HEXDUMP_TABLE = new char[1024];

    public static String hexDump(ByteBuf buffer) {
        return ByteBufUtil.hexDump(buffer, buffer.readerIndex(), buffer.readableBytes());
    }

    public static String hexDump(ByteBuf buffer, int fromIndex, int length) {
        if (length < 0) {
            throw new IllegalArgumentException("length: " + length);
        }
        if (length == 0) {
            return "";
        }
        int endIndex = fromIndex + length;
        char[] buf = new char[length << 1];
        int srcIdx = fromIndex;
        int dstIdx = 0;
        while (srcIdx < endIndex) {
            System.arraycopy(HEXDUMP_TABLE, buffer.getUnsignedByte(srcIdx) << 1, buf, dstIdx, 2);
            ++srcIdx;
            dstIdx += 2;
        }
        return new String(buf);
    }

    public static int hashCode(ByteBuf buffer) {
        int i2;
        int aLen = buffer.readableBytes();
        int intCount = aLen >>> 2;
        int byteCount = aLen & 3;
        int hashCode = 1;
        int arrayIndex = buffer.readerIndex();
        if (buffer.order() == ByteOrder.BIG_ENDIAN) {
            for (i2 = intCount; i2 > 0; --i2) {
                hashCode = 31 * hashCode + buffer.getInt(arrayIndex);
                arrayIndex += 4;
            }
        } else {
            for (i2 = intCount; i2 > 0; --i2) {
                hashCode = 31 * hashCode + ByteBufUtil.swapInt(buffer.getInt(arrayIndex));
                arrayIndex += 4;
            }
        }
        for (i2 = byteCount; i2 > 0; --i2) {
            hashCode = 31 * hashCode + buffer.getByte(arrayIndex++);
        }
        if (hashCode == 0) {
            hashCode = 1;
        }
        return hashCode;
    }

    public static boolean equals(ByteBuf bufferA, ByteBuf bufferB) {
        int i2;
        int aLen = bufferA.readableBytes();
        if (aLen != bufferB.readableBytes()) {
            return false;
        }
        int longCount = aLen >>> 3;
        int byteCount = aLen & 7;
        int aIndex = bufferA.readerIndex();
        int bIndex = bufferB.readerIndex();
        if (bufferA.order() == bufferB.order()) {
            for (i2 = longCount; i2 > 0; --i2) {
                if (bufferA.getLong(aIndex) != bufferB.getLong(bIndex)) {
                    return false;
                }
                aIndex += 8;
                bIndex += 8;
            }
        } else {
            for (i2 = longCount; i2 > 0; --i2) {
                if (bufferA.getLong(aIndex) != ByteBufUtil.swapLong(bufferB.getLong(bIndex))) {
                    return false;
                }
                aIndex += 8;
                bIndex += 8;
            }
        }
        for (i2 = byteCount; i2 > 0; --i2) {
            if (bufferA.getByte(aIndex) != bufferB.getByte(bIndex)) {
                return false;
            }
            ++aIndex;
            ++bIndex;
        }
        return true;
    }

    public static int compare(ByteBuf bufferA, ByteBuf bufferB) {
        long vb2;
        long va2;
        int i2;
        int aLen = bufferA.readableBytes();
        int bLen = bufferB.readableBytes();
        int minLength = Math.min(aLen, bLen);
        int uintCount = minLength >>> 2;
        int byteCount = minLength & 3;
        int aIndex = bufferA.readerIndex();
        int bIndex = bufferB.readerIndex();
        if (bufferA.order() == bufferB.order()) {
            for (i2 = uintCount; i2 > 0; --i2) {
                va2 = bufferA.getUnsignedInt(aIndex);
                if (va2 > (vb2 = bufferB.getUnsignedInt(bIndex))) {
                    return 1;
                }
                if (va2 < vb2) {
                    return -1;
                }
                aIndex += 4;
                bIndex += 4;
            }
        } else {
            for (i2 = uintCount; i2 > 0; --i2) {
                va2 = bufferA.getUnsignedInt(aIndex);
                if (va2 > (vb2 = (long)ByteBufUtil.swapInt(bufferB.getInt(bIndex)) & 0xFFFFFFFFL)) {
                    return 1;
                }
                if (va2 < vb2) {
                    return -1;
                }
                aIndex += 4;
                bIndex += 4;
            }
        }
        for (i2 = byteCount; i2 > 0; --i2) {
            short vb3;
            short va3 = bufferA.getUnsignedByte(aIndex);
            if (va3 > (vb3 = bufferB.getUnsignedByte(bIndex))) {
                return 1;
            }
            if (va3 < vb3) {
                return -1;
            }
            ++aIndex;
            ++bIndex;
        }
        return aLen - bLen;
    }

    public static int indexOf(ByteBuf buffer, int fromIndex, int toIndex, byte value) {
        if (fromIndex <= toIndex) {
            return ByteBufUtil.firstIndexOf(buffer, fromIndex, toIndex, value);
        }
        return ByteBufUtil.lastIndexOf(buffer, fromIndex, toIndex, value);
    }

    public static short swapShort(short value) {
        return Short.reverseBytes(value);
    }

    public static int swapMedium(int value) {
        int swapped = value << 16 & 0xFF0000 | value & 0xFF00 | value >>> 16 & 0xFF;
        if ((swapped & 0x800000) != 0) {
            swapped |= 0xFF000000;
        }
        return swapped;
    }

    public static int swapInt(int value) {
        return Integer.reverseBytes(value);
    }

    public static long swapLong(long value) {
        return Long.reverseBytes(value);
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    public static ByteBuf readBytes(ByteBufAllocator alloc, ByteBuf buffer, int length) {
        boolean release = true;
        ByteBuf dst = alloc.buffer(length);
        try {
            buffer.readBytes(dst);
            release = false;
            ByteBuf byteBuf = dst;
            return byteBuf;
        }
        finally {
            if (release) {
                dst.release();
            }
        }
    }

    private static int firstIndexOf(ByteBuf buffer, int fromIndex, int toIndex, byte value) {
        if ((fromIndex = Math.max(fromIndex, 0)) >= toIndex || buffer.capacity() == 0) {
            return -1;
        }
        for (int i2 = fromIndex; i2 < toIndex; ++i2) {
            if (buffer.getByte(i2) != value) continue;
            return i2;
        }
        return -1;
    }

    private static int lastIndexOf(ByteBuf buffer, int fromIndex, int toIndex, byte value) {
        if ((fromIndex = Math.min(fromIndex, buffer.capacity())) < 0 || buffer.capacity() == 0) {
            return -1;
        }
        for (int i2 = fromIndex - 1; i2 >= toIndex; --i2) {
            if (buffer.getByte(i2) != value) continue;
            return i2;
        }
        return -1;
    }

    static ByteBuffer encodeString(CharBuffer src, Charset charset) {
        CharsetEncoder encoder = CharsetUtil.getEncoder(charset);
        ByteBuffer dst = ByteBuffer.allocate((int)((double)src.remaining() * (double)encoder.maxBytesPerChar()));
        try {
            CoderResult cr2 = encoder.encode(src, dst, true);
            if (!cr2.isUnderflow()) {
                cr2.throwException();
            }
            if (!(cr2 = encoder.flush(dst)).isUnderflow()) {
                cr2.throwException();
            }
        }
        catch (CharacterCodingException x2) {
            throw new IllegalStateException(x2);
        }
        dst.flip();
        return dst;
    }

    static String decodeString(ByteBuffer src, Charset charset) {
        CharsetDecoder decoder = CharsetUtil.getDecoder(charset);
        CharBuffer dst = CharBuffer.allocate((int)((double)src.remaining() * (double)decoder.maxCharsPerByte()));
        try {
            CoderResult cr2 = decoder.decode(src, dst, true);
            if (!cr2.isUnderflow()) {
                cr2.throwException();
            }
            if (!(cr2 = decoder.flush(dst)).isUnderflow()) {
                cr2.throwException();
            }
        }
        catch (CharacterCodingException x2) {
            throw new IllegalStateException(x2);
        }
        return dst.flip().toString();
    }

    private ByteBufUtil() {
    }

    static {
        char[] DIGITS = "0123456789abcdef".toCharArray();
        for (int i2 = 0; i2 < 256; ++i2) {
            ByteBufUtil.HEXDUMP_TABLE[i2 << 1] = DIGITS[i2 >>> 4 & 0xF];
            ByteBufUtil.HEXDUMP_TABLE[(i2 << 1) + 1] = DIGITS[i2 & 0xF];
        }
    }
}

