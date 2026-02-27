/*
 * Decompiled with CFR 0.152.
 */
package io.netty.handler.codec.spdy;

import io.netty.buffer.ByteBuf;
import io.netty.handler.codec.spdy.SpdyCodecUtil;
import io.netty.handler.codec.spdy.SpdyHeaderBlockDecoder;
import io.netty.handler.codec.spdy.SpdyHeadersFrame;

public class SpdyHeaderBlockRawDecoder
extends SpdyHeaderBlockDecoder {
    private final int version;
    private final int maxHeaderSize;
    private final int lengthFieldSize;
    private int headerSize;
    private int numHeaders;

    public SpdyHeaderBlockRawDecoder(int version, int maxHeaderSize) {
        if (version < 2 || version > 3) {
            throw new IllegalArgumentException("unsupported version: " + version);
        }
        this.version = version;
        this.maxHeaderSize = maxHeaderSize;
        this.lengthFieldSize = version < 3 ? 2 : 4;
        this.reset();
    }

    private int readLengthField(ByteBuf buffer) {
        int length;
        if (this.version < 3) {
            length = SpdyCodecUtil.getUnsignedShort(buffer, buffer.readerIndex());
            buffer.skipBytes(2);
        } else {
            length = SpdyCodecUtil.getSignedInt(buffer, buffer.readerIndex());
            buffer.skipBytes(4);
        }
        return length;
    }

    @Override
    void decode(ByteBuf encoded, SpdyHeadersFrame frame) throws Exception {
        if (encoded == null) {
            throw new NullPointerException("encoded");
        }
        if (frame == null) {
            throw new NullPointerException("frame");
        }
        if (this.numHeaders == -1) {
            if (encoded.readableBytes() < this.lengthFieldSize) {
                return;
            }
            this.numHeaders = this.readLengthField(encoded);
            if (this.numHeaders < 0) {
                frame.setInvalid();
                return;
            }
        }
        while (this.numHeaders > 0) {
            int headerSize = this.headerSize;
            encoded.markReaderIndex();
            if (encoded.readableBytes() < this.lengthFieldSize) {
                encoded.resetReaderIndex();
                return;
            }
            int nameLength = this.readLengthField(encoded);
            if (nameLength <= 0) {
                frame.setInvalid();
                return;
            }
            if ((headerSize += nameLength) > this.maxHeaderSize) {
                frame.setTruncated();
                return;
            }
            if (encoded.readableBytes() < nameLength) {
                encoded.resetReaderIndex();
                return;
            }
            byte[] nameBytes = new byte[nameLength];
            encoded.readBytes(nameBytes);
            String name = new String(nameBytes, "UTF-8");
            if (frame.headers().contains(name)) {
                frame.setInvalid();
                return;
            }
            if (encoded.readableBytes() < this.lengthFieldSize) {
                encoded.resetReaderIndex();
                return;
            }
            int valueLength = this.readLengthField(encoded);
            if (valueLength < 0) {
                frame.setInvalid();
                return;
            }
            if (valueLength == 0) {
                if (this.version < 3) {
                    frame.setInvalid();
                    return;
                }
                frame.headers().add(name, "");
                --this.numHeaders;
                this.headerSize = headerSize;
                continue;
            }
            if ((headerSize += valueLength) > this.maxHeaderSize) {
                frame.setTruncated();
                return;
            }
            if (encoded.readableBytes() < valueLength) {
                encoded.resetReaderIndex();
                return;
            }
            byte[] valueBytes = new byte[valueLength];
            encoded.readBytes(valueBytes);
            int index = 0;
            int offset = 0;
            while (index < valueLength) {
                while (index < valueBytes.length && valueBytes[index] != 0) {
                    ++index;
                }
                if (index < valueBytes.length && valueBytes[index + 1] == 0) {
                    frame.setInvalid();
                    return;
                }
                String value = new String(valueBytes, offset, index - offset, "UTF-8");
                try {
                    frame.headers().add(name, value);
                }
                catch (IllegalArgumentException e2) {
                    frame.setInvalid();
                    return;
                }
                offset = ++index;
            }
            --this.numHeaders;
            this.headerSize = headerSize;
        }
    }

    @Override
    void reset() {
        this.headerSize = 0;
        this.numHeaders = -1;
    }

    @Override
    void end() {
    }
}

