/*
 * Decompiled with CFR 0.152.
 */
package io.netty.handler.codec.http;

import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufUtil;
import io.netty.buffer.Unpooled;
import io.netty.channel.ChannelHandlerContext;
import io.netty.handler.codec.DecoderResult;
import io.netty.handler.codec.ReplayingDecoder;
import io.netty.handler.codec.TooLongFrameException;
import io.netty.handler.codec.http.DefaultHttpContent;
import io.netty.handler.codec.http.DefaultLastHttpContent;
import io.netty.handler.codec.http.HttpContent;
import io.netty.handler.codec.http.HttpHeaders;
import io.netty.handler.codec.http.HttpMessage;
import io.netty.handler.codec.http.HttpResponse;
import io.netty.handler.codec.http.LastHttpContent;
import java.util.List;

public abstract class HttpObjectDecoder
extends ReplayingDecoder<State> {
    private static final ThreadLocal<StringBuilder> BUILDERS = new ThreadLocal<StringBuilder>(){

        @Override
        protected StringBuilder initialValue() {
            return new StringBuilder(512);
        }

        @Override
        public StringBuilder get() {
            StringBuilder builder = (StringBuilder)super.get();
            builder.setLength(0);
            return builder;
        }
    };
    private final int maxInitialLineLength;
    private final int maxHeaderSize;
    private final int maxChunkSize;
    private final boolean chunkedSupported;
    private ByteBuf content;
    private HttpMessage message;
    private long chunkSize;
    private int headerSize;
    private int contentRead;

    protected HttpObjectDecoder() {
        this(4096, 8192, 8192, true);
    }

    protected HttpObjectDecoder(int maxInitialLineLength, int maxHeaderSize, int maxChunkSize, boolean chunkedSupported) {
        super(State.SKIP_CONTROL_CHARS);
        if (maxInitialLineLength <= 0) {
            throw new IllegalArgumentException("maxInitialLineLength must be a positive integer: " + maxInitialLineLength);
        }
        if (maxHeaderSize <= 0) {
            throw new IllegalArgumentException("maxHeaderSize must be a positive integer: " + maxHeaderSize);
        }
        if (maxChunkSize < 0) {
            throw new IllegalArgumentException("maxChunkSize must be a positive integer: " + maxChunkSize);
        }
        this.maxInitialLineLength = maxInitialLineLength;
        this.maxHeaderSize = maxHeaderSize;
        this.maxChunkSize = maxChunkSize;
        this.chunkedSupported = chunkedSupported;
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    @Override
    protected void decode(ChannelHandlerContext ctx, ByteBuf buffer, List<Object> out) throws Exception {
        switch ((State)((Object)this.state())) {
            case SKIP_CONTROL_CHARS: {
                try {
                    HttpObjectDecoder.skipControlCharacters(buffer);
                    this.checkpoint(State.READ_INITIAL);
                }
                finally {
                    this.checkpoint();
                }
            }
            case READ_INITIAL: {
                try {
                    String[] initialLine = HttpObjectDecoder.splitInitialLine(HttpObjectDecoder.readLine(buffer, this.maxInitialLineLength));
                    if (initialLine.length < 3) {
                        this.checkpoint(State.SKIP_CONTROL_CHARS);
                        return;
                    }
                    this.message = this.createMessage(initialLine);
                    this.checkpoint(State.READ_HEADER);
                }
                catch (Exception e2) {
                    out.add(this.invalidMessage(e2));
                    return;
                }
            }
            case READ_HEADER: {
                try {
                    State nextState = this.readHeaders(buffer);
                    this.checkpoint(nextState);
                    if (nextState == State.READ_CHUNK_SIZE) {
                        if (!this.chunkedSupported) {
                            throw new IllegalArgumentException("Chunked messages not supported");
                        }
                        out.add(this.message);
                        return;
                    }
                    if (nextState == State.SKIP_CONTROL_CHARS) {
                        this.reset(out);
                        return;
                    }
                    long contentLength = HttpHeaders.getContentLength(this.message, -1L);
                    if (contentLength == 0L || contentLength == -1L && this.isDecodingRequest()) {
                        this.content = Unpooled.EMPTY_BUFFER;
                        this.reset(out);
                        return;
                    }
                    switch (nextState) {
                        case READ_FIXED_LENGTH_CONTENT: {
                            if (contentLength <= (long)this.maxChunkSize && !HttpHeaders.is100ContinueExpected(this.message)) break;
                            this.checkpoint(State.READ_FIXED_LENGTH_CONTENT_AS_CHUNKS);
                            this.chunkSize = HttpHeaders.getContentLength(this.message, -1L);
                            out.add(this.message);
                            return;
                        }
                        case READ_VARIABLE_LENGTH_CONTENT: {
                            if (buffer.readableBytes() <= this.maxChunkSize && !HttpHeaders.is100ContinueExpected(this.message)) break;
                            this.checkpoint(State.READ_VARIABLE_LENGTH_CONTENT_AS_CHUNKS);
                            out.add(this.message);
                            return;
                        }
                        default: {
                            throw new IllegalStateException("Unexpected state: " + (Object)((Object)nextState));
                        }
                    }
                    return;
                }
                catch (Exception e3) {
                    out.add(this.invalidMessage(e3));
                    return;
                }
            }
            case READ_VARIABLE_LENGTH_CONTENT: {
                int toRead = this.actualReadableBytes();
                if (toRead > this.maxChunkSize) {
                    toRead = this.maxChunkSize;
                }
                out.add(this.message);
                out.add(new DefaultHttpContent(ByteBufUtil.readBytes(ctx.alloc(), buffer, toRead)));
                return;
            }
            case READ_VARIABLE_LENGTH_CONTENT_AS_CHUNKS: {
                int toRead = this.actualReadableBytes();
                if (toRead > this.maxChunkSize) {
                    toRead = this.maxChunkSize;
                }
                ByteBuf content = ByteBufUtil.readBytes(ctx.alloc(), buffer, toRead);
                if (!buffer.isReadable()) {
                    this.reset();
                    out.add(new DefaultLastHttpContent(content));
                    return;
                }
                out.add(new DefaultHttpContent(content));
                return;
            }
            case READ_FIXED_LENGTH_CONTENT: {
                this.readFixedLengthContent(ctx, buffer, out);
                return;
            }
            case READ_FIXED_LENGTH_CONTENT_AS_CHUNKS: {
                long chunkSize = this.chunkSize;
                int readLimit = this.actualReadableBytes();
                if (readLimit == 0) {
                    return;
                }
                int toRead = readLimit;
                if (toRead > this.maxChunkSize) {
                    toRead = this.maxChunkSize;
                }
                if ((long)toRead > chunkSize) {
                    toRead = (int)chunkSize;
                }
                ByteBuf content = ByteBufUtil.readBytes(ctx.alloc(), buffer, toRead);
                chunkSize = chunkSize > (long)toRead ? (chunkSize -= (long)toRead) : 0L;
                this.chunkSize = chunkSize;
                if (chunkSize == 0L) {
                    this.reset();
                    out.add(new DefaultLastHttpContent(content));
                    return;
                }
                out.add(new DefaultHttpContent(content));
                return;
            }
            case READ_CHUNK_SIZE: {
                try {
                    StringBuilder line = HttpObjectDecoder.readLine(buffer, this.maxInitialLineLength);
                    int chunkSize = HttpObjectDecoder.getChunkSize(line.toString());
                    this.chunkSize = chunkSize;
                    if (chunkSize == 0) {
                        this.checkpoint(State.READ_CHUNK_FOOTER);
                        return;
                    }
                    if (chunkSize > this.maxChunkSize) {
                        this.checkpoint(State.READ_CHUNKED_CONTENT_AS_CHUNKS);
                    } else {
                        this.checkpoint(State.READ_CHUNKED_CONTENT);
                    }
                }
                catch (Exception e4) {
                    out.add(this.invalidChunk(e4));
                    return;
                }
            }
            case READ_CHUNKED_CONTENT: {
                assert (this.chunkSize <= Integer.MAX_VALUE);
                DefaultHttpContent chunk = new DefaultHttpContent(ByteBufUtil.readBytes(ctx.alloc(), buffer, (int)this.chunkSize));
                this.checkpoint(State.READ_CHUNK_DELIMITER);
                out.add(chunk);
                return;
            }
            case READ_CHUNKED_CONTENT_AS_CHUNKS: {
                assert (this.chunkSize <= Integer.MAX_VALUE);
                int chunkSize = (int)this.chunkSize;
                int readLimit = this.actualReadableBytes();
                if (readLimit == 0) {
                    return;
                }
                int toRead = chunkSize;
                if (toRead > this.maxChunkSize) {
                    toRead = this.maxChunkSize;
                }
                if (toRead > readLimit) {
                    toRead = readLimit;
                }
                DefaultHttpContent chunk = new DefaultHttpContent(ByteBufUtil.readBytes(ctx.alloc(), buffer, toRead));
                chunkSize = chunkSize > toRead ? (chunkSize -= toRead) : 0;
                this.chunkSize = chunkSize;
                if (chunkSize == 0) {
                    this.checkpoint(State.READ_CHUNK_DELIMITER);
                }
                out.add(chunk);
                return;
            }
            case READ_CHUNK_DELIMITER: {
                while (true) {
                    byte next;
                    if ((next = buffer.readByte()) == 13) {
                        if (buffer.readByte() != 10) continue;
                        this.checkpoint(State.READ_CHUNK_SIZE);
                        return;
                    }
                    if (next == 10) {
                        this.checkpoint(State.READ_CHUNK_SIZE);
                        return;
                    }
                    this.checkpoint();
                }
            }
            case READ_CHUNK_FOOTER: {
                try {
                    LastHttpContent trailer = this.readTrailingHeaders(buffer);
                    if (this.maxChunkSize == 0) {
                        this.reset(out);
                        return;
                    }
                    this.reset();
                    out.add(trailer);
                    return;
                }
                catch (Exception e5) {
                    out.add(this.invalidChunk(e5));
                    return;
                }
            }
            case BAD_MESSAGE: {
                buffer.skipBytes(this.actualReadableBytes());
                return;
            }
        }
        throw new Error("Shouldn't reach here.");
    }

    @Override
    protected void decodeLast(ChannelHandlerContext ctx, ByteBuf in2, List<Object> out) throws Exception {
        this.decode(ctx, in2, out);
        if (this.message != null) {
            boolean prematureClosure;
            HttpMessage message = this.message;
            int actualContentLength = this.content != null ? this.content.readableBytes() : 0;
            if (this.isDecodingRequest()) {
                prematureClosure = true;
            } else {
                long expectedContentLength = HttpHeaders.getContentLength(message, -1L);
                boolean bl2 = prematureClosure = expectedContentLength >= 0L && (long)actualContentLength != expectedContentLength;
            }
            if (!prematureClosure) {
                if (actualContentLength == 0) {
                    out.add(LastHttpContent.EMPTY_LAST_CONTENT);
                } else {
                    out.add(new DefaultLastHttpContent(this.content));
                }
            }
        }
    }

    protected boolean isContentAlwaysEmpty(HttpMessage msg) {
        if (msg instanceof HttpResponse) {
            HttpResponse res = (HttpResponse)msg;
            int code = res.getStatus().code();
            if (code >= 100 && code < 200) {
                return code != 101 || res.headers().contains("Sec-WebSocket-Accept");
            }
            switch (code) {
                case 204: 
                case 205: 
                case 304: {
                    return true;
                }
            }
        }
        return false;
    }

    private void reset() {
        this.reset(null);
    }

    private void reset(List<Object> out) {
        if (out != null) {
            HttpMessage message = this.message;
            ByteBuf content = this.content;
            LastHttpContent httpContent = content == null || !content.isReadable() ? LastHttpContent.EMPTY_LAST_CONTENT : new DefaultLastHttpContent(content);
            out.add(message);
            out.add(httpContent);
        }
        this.content = null;
        this.message = null;
        this.checkpoint(State.SKIP_CONTROL_CHARS);
    }

    private HttpMessage invalidMessage(Exception cause) {
        this.checkpoint(State.BAD_MESSAGE);
        if (this.message != null) {
            this.message.setDecoderResult(DecoderResult.failure(cause));
        } else {
            this.message = this.createInvalidMessage();
            this.message.setDecoderResult(DecoderResult.failure(cause));
        }
        return this.message;
    }

    private HttpContent invalidChunk(Exception cause) {
        this.checkpoint(State.BAD_MESSAGE);
        DefaultHttpContent chunk = new DefaultHttpContent(Unpooled.EMPTY_BUFFER);
        chunk.setDecoderResult(DecoderResult.failure(cause));
        return chunk;
    }

    private static void skipControlCharacters(ByteBuf buffer) {
        char c2;
        while (Character.isISOControl(c2 = (char)buffer.readUnsignedByte()) || Character.isWhitespace(c2)) {
        }
        buffer.readerIndex(buffer.readerIndex() - 1);
    }

    private void readFixedLengthContent(ChannelHandlerContext ctx, ByteBuf buffer, List<Object> out) {
        long length = HttpHeaders.getContentLength(this.message, -1L);
        assert (length <= Integer.MAX_VALUE);
        int toRead = (int)length - this.contentRead;
        if (toRead > this.actualReadableBytes()) {
            toRead = this.actualReadableBytes();
        }
        this.contentRead += toRead;
        if (length < (long)this.contentRead) {
            out.add(this.message);
            out.add(new DefaultHttpContent(ByteBufUtil.readBytes(ctx.alloc(), buffer, toRead)));
            return;
        }
        if (this.content == null) {
            this.content = ByteBufUtil.readBytes(ctx.alloc(), buffer, (int)length);
        } else {
            this.content.writeBytes(buffer, (int)length);
        }
        this.reset(out);
    }

    private State readHeaders(ByteBuf buffer) {
        State nextState;
        this.headerSize = 0;
        HttpMessage message = this.message;
        HttpHeaders headers = message.headers();
        StringBuilder line = this.readHeader(buffer);
        String name = null;
        String value = null;
        if (line.length() > 0) {
            headers.clear();
            do {
                char firstChar = line.charAt(0);
                if (name != null && (firstChar == ' ' || firstChar == '\t')) {
                    value = value + ' ' + line.toString().trim();
                    continue;
                }
                if (name != null) {
                    headers.add(name, value);
                }
                String[] header = HttpObjectDecoder.splitHeader(line);
                name = header[0];
                value = header[1];
            } while ((line = this.readHeader(buffer)).length() > 0);
            if (name != null) {
                headers.add(name, value);
            }
        }
        if (this.isContentAlwaysEmpty(message)) {
            HttpHeaders.removeTransferEncodingChunked(message);
            nextState = State.SKIP_CONTROL_CHARS;
        } else {
            nextState = HttpHeaders.isTransferEncodingChunked(message) ? State.READ_CHUNK_SIZE : (HttpHeaders.getContentLength(message, -1L) >= 0L ? State.READ_FIXED_LENGTH_CONTENT : State.READ_VARIABLE_LENGTH_CONTENT);
        }
        return nextState;
    }

    private LastHttpContent readTrailingHeaders(ByteBuf buffer) {
        this.headerSize = 0;
        StringBuilder line = this.readHeader(buffer);
        String lastHeader = null;
        if (line.length() > 0) {
            DefaultLastHttpContent trailer = new DefaultLastHttpContent(Unpooled.EMPTY_BUFFER);
            do {
                char firstChar = line.charAt(0);
                if (lastHeader != null && (firstChar == ' ' || firstChar == '\t')) {
                    List<String> current = trailer.trailingHeaders().getAll(lastHeader);
                    if (current.isEmpty()) continue;
                    int lastPos = current.size() - 1;
                    String newString = current.get(lastPos) + line.toString().trim();
                    current.set(lastPos, newString);
                    continue;
                }
                String[] header = HttpObjectDecoder.splitHeader(line);
                String name = header[0];
                if (!(name.equalsIgnoreCase("Content-Length") || name.equalsIgnoreCase("Transfer-Encoding") || name.equalsIgnoreCase("Trailer"))) {
                    trailer.trailingHeaders().add(name, header[1]);
                }
                lastHeader = name;
            } while ((line = this.readHeader(buffer)).length() > 0);
            return trailer;
        }
        return LastHttpContent.EMPTY_LAST_CONTENT;
    }

    /*
     * Enabled aggressive block sorting
     */
    private StringBuilder readHeader(ByteBuf buffer) {
        StringBuilder sb2 = BUILDERS.get();
        int headerSize = this.headerSize;
        block4: while (true) {
            char nextByte = (char)buffer.readByte();
            ++headerSize;
            switch (nextByte) {
                case '\r': {
                    nextByte = (char)buffer.readByte();
                    ++headerSize;
                    if (nextByte != '\n') break;
                    break block4;
                }
                case '\n': {
                    break block4;
                }
            }
            if (headerSize >= this.maxHeaderSize) {
                throw new TooLongFrameException("HTTP header is larger than " + this.maxHeaderSize + " bytes.");
            }
            sb2.append(nextByte);
        }
        this.headerSize = headerSize;
        return sb2;
    }

    protected abstract boolean isDecodingRequest();

    protected abstract HttpMessage createMessage(String[] var1) throws Exception;

    protected abstract HttpMessage createInvalidMessage();

    private static int getChunkSize(String hex) {
        hex = hex.trim();
        for (int i2 = 0; i2 < hex.length(); ++i2) {
            char c2 = hex.charAt(i2);
            if (c2 != ';' && !Character.isWhitespace(c2) && !Character.isISOControl(c2)) continue;
            hex = hex.substring(0, i2);
            break;
        }
        return Integer.parseInt(hex, 16);
    }

    private static StringBuilder readLine(ByteBuf buffer, int maxLineLength) {
        StringBuilder sb2 = BUILDERS.get();
        int lineLength = 0;
        while (true) {
            byte nextByte;
            if ((nextByte = buffer.readByte()) == 13) {
                nextByte = buffer.readByte();
                if (nextByte != 10) continue;
                return sb2;
            }
            if (nextByte == 10) {
                return sb2;
            }
            if (lineLength >= maxLineLength) {
                throw new TooLongFrameException("An HTTP line is larger than " + maxLineLength + " bytes.");
            }
            ++lineLength;
            sb2.append((char)nextByte);
        }
    }

    private static String[] splitInitialLine(StringBuilder sb2) {
        int aStart = HttpObjectDecoder.findNonWhitespace(sb2, 0);
        int aEnd = HttpObjectDecoder.findWhitespace(sb2, aStart);
        int bStart = HttpObjectDecoder.findNonWhitespace(sb2, aEnd);
        int bEnd = HttpObjectDecoder.findWhitespace(sb2, bStart);
        int cStart = HttpObjectDecoder.findNonWhitespace(sb2, bEnd);
        int cEnd = HttpObjectDecoder.findEndOfString(sb2);
        return new String[]{sb2.substring(aStart, aEnd), sb2.substring(bStart, bEnd), cStart < cEnd ? sb2.substring(cStart, cEnd) : ""};
    }

    private static String[] splitHeader(StringBuilder sb2) {
        int valueStart;
        int colonEnd;
        int nameStart;
        char ch2;
        int nameEnd;
        int length = sb2.length();
        for (nameEnd = nameStart = HttpObjectDecoder.findNonWhitespace(sb2, 0); nameEnd < length && (ch2 = sb2.charAt(nameEnd)) != ':' && !Character.isWhitespace(ch2); ++nameEnd) {
        }
        for (colonEnd = nameEnd; colonEnd < length; ++colonEnd) {
            if (sb2.charAt(colonEnd) != ':') continue;
            ++colonEnd;
            break;
        }
        if ((valueStart = HttpObjectDecoder.findNonWhitespace(sb2, colonEnd)) == length) {
            return new String[]{sb2.substring(nameStart, nameEnd), ""};
        }
        int valueEnd = HttpObjectDecoder.findEndOfString(sb2);
        return new String[]{sb2.substring(nameStart, nameEnd), sb2.substring(valueStart, valueEnd)};
    }

    private static int findNonWhitespace(CharSequence sb2, int offset) {
        int result;
        for (result = offset; result < sb2.length() && Character.isWhitespace(sb2.charAt(result)); ++result) {
        }
        return result;
    }

    private static int findWhitespace(CharSequence sb2, int offset) {
        int result;
        for (result = offset; result < sb2.length() && !Character.isWhitespace(sb2.charAt(result)); ++result) {
        }
        return result;
    }

    private static int findEndOfString(CharSequence sb2) {
        int result;
        for (result = sb2.length(); result > 0 && Character.isWhitespace(sb2.charAt(result - 1)); --result) {
        }
        return result;
    }

    static enum State {
        SKIP_CONTROL_CHARS,
        READ_INITIAL,
        READ_HEADER,
        READ_VARIABLE_LENGTH_CONTENT,
        READ_VARIABLE_LENGTH_CONTENT_AS_CHUNKS,
        READ_FIXED_LENGTH_CONTENT,
        READ_FIXED_LENGTH_CONTENT_AS_CHUNKS,
        READ_CHUNK_SIZE,
        READ_CHUNKED_CONTENT,
        READ_CHUNKED_CONTENT_AS_CHUNKS,
        READ_CHUNK_DELIMITER,
        READ_CHUNK_FOOTER,
        BAD_MESSAGE;

    }
}

