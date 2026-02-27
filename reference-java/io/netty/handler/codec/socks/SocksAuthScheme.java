/*
 * Decompiled with CFR 0.152.
 */
package io.netty.handler.codec.socks;

public enum SocksAuthScheme {
    NO_AUTH(0),
    AUTH_GSSAPI(1),
    AUTH_PASSWORD(2),
    UNKNOWN(-1);

    private final byte b;

    private SocksAuthScheme(byte b2) {
        this.b = b2;
    }

    public static SocksAuthScheme fromByte(byte b2) {
        for (SocksAuthScheme code : SocksAuthScheme.values()) {
            if (code.b != b2) continue;
            return code;
        }
        return UNKNOWN;
    }

    public byte byteValue() {
        return this.b;
    }
}

