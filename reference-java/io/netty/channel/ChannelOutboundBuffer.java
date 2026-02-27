/*
 * Decompiled with CFR 0.152.
 */
package io.netty.channel;

import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufAllocator;
import io.netty.buffer.ByteBufHolder;
import io.netty.channel.AbstractChannel;
import io.netty.channel.ChannelProgressivePromise;
import io.netty.channel.ChannelPromise;
import io.netty.channel.FileRegion;
import io.netty.channel.VoidChannelPromise;
import io.netty.util.Recycler;
import io.netty.util.ReferenceCountUtil;
import io.netty.util.internal.logging.InternalLogger;
import io.netty.util.internal.logging.InternalLoggerFactory;
import java.nio.ByteBuffer;
import java.nio.channels.ClosedChannelException;
import java.util.Arrays;
import java.util.concurrent.atomic.AtomicIntegerFieldUpdater;
import java.util.concurrent.atomic.AtomicLongFieldUpdater;

public final class ChannelOutboundBuffer {
    private static final InternalLogger logger = InternalLoggerFactory.getInstance(ChannelOutboundBuffer.class);
    private static final int INITIAL_CAPACITY = 32;
    private static final Recycler<ChannelOutboundBuffer> RECYCLER = new Recycler<ChannelOutboundBuffer>(){

        @Override
        protected ChannelOutboundBuffer newObject(Recycler.Handle handle) {
            return new ChannelOutboundBuffer(handle);
        }
    };
    private final Recycler.Handle handle;
    private AbstractChannel channel;
    private Entry[] buffer;
    private int flushed;
    private int unflushed;
    private int tail;
    private ByteBuffer[] nioBuffers;
    private int nioBufferCount;
    private long nioBufferSize;
    private boolean inFail;
    private static final AtomicLongFieldUpdater<ChannelOutboundBuffer> TOTAL_PENDING_SIZE_UPDATER = AtomicLongFieldUpdater.newUpdater(ChannelOutboundBuffer.class, "totalPendingSize");
    private volatile long totalPendingSize;
    private static final AtomicIntegerFieldUpdater<ChannelOutboundBuffer> WRITABLE_UPDATER = AtomicIntegerFieldUpdater.newUpdater(ChannelOutboundBuffer.class, "writable");
    private volatile int writable = 1;

    static ChannelOutboundBuffer newInstance(AbstractChannel channel) {
        ChannelOutboundBuffer buffer = RECYCLER.get();
        buffer.channel = channel;
        buffer.totalPendingSize = 0L;
        buffer.writable = 1;
        return buffer;
    }

    private ChannelOutboundBuffer(Recycler.Handle handle) {
        this.handle = handle;
        this.buffer = new Entry[32];
        for (int i2 = 0; i2 < this.buffer.length; ++i2) {
            this.buffer[i2] = new Entry();
        }
        this.nioBuffers = new ByteBuffer[32];
    }

    void addMessage(Object msg, ChannelPromise promise) {
        int size = this.channel.estimatorHandle().size(msg);
        if (size < 0) {
            size = 0;
        }
        Entry e2 = this.buffer[this.tail++];
        e2.msg = msg;
        e2.pendingSize = size;
        e2.promise = promise;
        e2.total = ChannelOutboundBuffer.total(msg);
        this.tail &= this.buffer.length - 1;
        if (this.tail == this.flushed) {
            this.addCapacity();
        }
        this.incrementPendingOutboundBytes(size);
    }

    private void addCapacity() {
        int p2 = this.flushed;
        int n2 = this.buffer.length;
        int r2 = n2 - p2;
        int s2 = this.size();
        int newCapacity = n2 << 1;
        if (newCapacity < 0) {
            throw new IllegalStateException();
        }
        Entry[] e2 = new Entry[newCapacity];
        System.arraycopy(this.buffer, p2, e2, 0, r2);
        System.arraycopy(this.buffer, 0, e2, r2, p2);
        for (int i2 = n2; i2 < e2.length; ++i2) {
            e2[i2] = new Entry();
        }
        this.buffer = e2;
        this.flushed = 0;
        this.unflushed = s2;
        this.tail = n2;
    }

    void addFlush() {
        this.unflushed = this.tail;
    }

    void incrementPendingOutboundBytes(int size) {
        AbstractChannel channel = this.channel;
        if (size == 0 || channel == null) {
            return;
        }
        long oldValue = this.totalPendingSize;
        long newWriteBufferSize = oldValue + (long)size;
        while (!TOTAL_PENDING_SIZE_UPDATER.compareAndSet(this, oldValue, newWriteBufferSize)) {
            oldValue = this.totalPendingSize;
            newWriteBufferSize = oldValue + (long)size;
        }
        int highWaterMark = channel.config().getWriteBufferHighWaterMark();
        if (newWriteBufferSize > (long)highWaterMark && WRITABLE_UPDATER.compareAndSet(this, 1, 0)) {
            channel.pipeline().fireChannelWritabilityChanged();
        }
    }

    void decrementPendingOutboundBytes(int size) {
        AbstractChannel channel = this.channel;
        if (size == 0 || channel == null) {
            return;
        }
        long oldValue = this.totalPendingSize;
        long newWriteBufferSize = oldValue - (long)size;
        while (!TOTAL_PENDING_SIZE_UPDATER.compareAndSet(this, oldValue, newWriteBufferSize)) {
            oldValue = this.totalPendingSize;
            newWriteBufferSize = oldValue - (long)size;
        }
        int lowWaterMark = channel.config().getWriteBufferLowWaterMark();
        if ((newWriteBufferSize == 0L || newWriteBufferSize < (long)lowWaterMark) && WRITABLE_UPDATER.compareAndSet(this, 0, 1)) {
            channel.pipeline().fireChannelWritabilityChanged();
        }
    }

    private static long total(Object msg) {
        if (msg instanceof ByteBuf) {
            return ((ByteBuf)msg).readableBytes();
        }
        if (msg instanceof FileRegion) {
            return ((FileRegion)msg).count();
        }
        if (msg instanceof ByteBufHolder) {
            return ((ByteBufHolder)msg).content().readableBytes();
        }
        return -1L;
    }

    public Object current() {
        if (this.isEmpty()) {
            return null;
        }
        return this.buffer[this.flushed].msg;
    }

    public void current(Object msg) {
        Entry entry = this.buffer[this.flushed];
        ChannelOutboundBuffer.safeRelease(entry.msg);
        entry.msg = msg;
    }

    public void progress(long amount) {
        Entry e2 = this.buffer[this.flushed];
        ChannelPromise p2 = e2.promise;
        if (p2 instanceof ChannelProgressivePromise) {
            long progress;
            e2.progress = progress = e2.progress + amount;
            ((ChannelProgressivePromise)p2).tryProgress(progress, e2.total);
        }
    }

    public boolean remove() {
        if (this.isEmpty()) {
            return false;
        }
        Entry e2 = this.buffer[this.flushed];
        Object msg = e2.msg;
        if (msg == null) {
            return false;
        }
        ChannelPromise promise = e2.promise;
        int size = e2.pendingSize;
        e2.clear();
        this.flushed = this.flushed + 1 & this.buffer.length - 1;
        ChannelOutboundBuffer.safeRelease(msg);
        promise.trySuccess();
        this.decrementPendingOutboundBytes(size);
        return true;
    }

    public boolean remove(Throwable cause) {
        if (this.isEmpty()) {
            return false;
        }
        Entry e2 = this.buffer[this.flushed];
        Object msg = e2.msg;
        if (msg == null) {
            return false;
        }
        ChannelPromise promise = e2.promise;
        int size = e2.pendingSize;
        e2.clear();
        this.flushed = this.flushed + 1 & this.buffer.length - 1;
        ChannelOutboundBuffer.safeRelease(msg);
        ChannelOutboundBuffer.safeFail(promise, cause);
        this.decrementPendingOutboundBytes(size);
        return true;
    }

    public ByteBuffer[] nioBuffers() {
        Object m2;
        long nioBufferSize = 0L;
        int nioBufferCount = 0;
        int mask = this.buffer.length - 1;
        ByteBufAllocator alloc = this.channel.alloc();
        ByteBuffer[] nioBuffers = this.nioBuffers;
        int i2 = this.flushed;
        while (i2 != this.unflushed && (m2 = this.buffer[i2].msg) != null) {
            if (!(m2 instanceof ByteBuf)) {
                this.nioBufferCount = 0;
                this.nioBufferSize = 0L;
                return null;
            }
            Entry entry = this.buffer[i2];
            ByteBuf buf = (ByteBuf)m2;
            int readerIndex = buf.readerIndex();
            int readableBytes = buf.writerIndex() - readerIndex;
            if (readableBytes > 0) {
                nioBufferSize += (long)readableBytes;
                int count = entry.count;
                if (count == -1) {
                    entry.count = count = buf.nioBufferCount();
                }
                if (nioBufferCount + count > nioBuffers.length) {
                    nioBuffers = ChannelOutboundBuffer.doubleNioBufferArray(nioBuffers, nioBufferCount);
                    this.nioBuffers = nioBuffers;
                }
                if (buf.isDirect() || !alloc.isDirectBufferPooled()) {
                    if (count == 1) {
                        ByteBuffer nioBuf = entry.buf;
                        if (nioBuf == null) {
                            entry.buf = nioBuf = buf.internalNioBuffer(readerIndex, readableBytes);
                        }
                        nioBuffers[nioBufferCount++] = nioBuf;
                    } else {
                        ByteBuffer[] nioBufs = entry.buffers;
                        if (nioBufs == null) {
                            nioBufs = buf.nioBuffers();
                            entry.buffers = nioBufs;
                        }
                        nioBufferCount = ChannelOutboundBuffer.fillBufferArray(nioBufs, nioBuffers, nioBufferCount);
                    }
                } else {
                    nioBufferCount = ChannelOutboundBuffer.fillBufferArrayNonDirect(entry, buf, readerIndex, readableBytes, alloc, nioBuffers, nioBufferCount);
                }
            }
            i2 = i2 + 1 & mask;
        }
        this.nioBufferCount = nioBufferCount;
        this.nioBufferSize = nioBufferSize;
        return nioBuffers;
    }

    private static int fillBufferArray(ByteBuffer[] nioBufs, ByteBuffer[] nioBuffers, int nioBufferCount) {
        for (ByteBuffer nioBuf : nioBufs) {
            if (nioBuf == null) break;
            nioBuffers[nioBufferCount++] = nioBuf;
        }
        return nioBufferCount;
    }

    private static int fillBufferArrayNonDirect(Entry entry, ByteBuf buf, int readerIndex, int readableBytes, ByteBufAllocator alloc, ByteBuffer[] nioBuffers, int nioBufferCount) {
        ByteBuf directBuf = alloc.directBuffer(readableBytes);
        directBuf.writeBytes(buf, readerIndex, readableBytes);
        buf.release();
        entry.msg = directBuf;
        ByteBuffer nioBuf = entry.buf = directBuf.internalNioBuffer(0, readableBytes);
        entry.count = 1;
        nioBuffers[nioBufferCount++] = nioBuf;
        return nioBufferCount;
    }

    private static ByteBuffer[] doubleNioBufferArray(ByteBuffer[] array, int size) {
        int newCapacity = array.length << 1;
        if (newCapacity < 0) {
            throw new IllegalStateException();
        }
        ByteBuffer[] newArray = new ByteBuffer[newCapacity];
        System.arraycopy(array, 0, newArray, 0, size);
        return newArray;
    }

    public int nioBufferCount() {
        return this.nioBufferCount;
    }

    public long nioBufferSize() {
        return this.nioBufferSize;
    }

    boolean getWritable() {
        return this.writable != 0;
    }

    public int size() {
        return this.unflushed - this.flushed & this.buffer.length - 1;
    }

    public boolean isEmpty() {
        return this.unflushed == this.flushed;
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    void failFlushed(Throwable cause) {
        if (this.inFail) {
            return;
        }
        try {
            this.inFail = true;
            while (this.remove(cause)) {
            }
        }
        finally {
            this.inFail = false;
        }
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    void close(final ClosedChannelException cause) {
        if (this.inFail) {
            this.channel.eventLoop().execute(new Runnable(){

                @Override
                public void run() {
                    ChannelOutboundBuffer.this.close(cause);
                }
            });
            return;
        }
        this.inFail = true;
        if (this.channel.isOpen()) {
            throw new IllegalStateException("close() must be invoked after the channel is closed.");
        }
        if (!this.isEmpty()) {
            throw new IllegalStateException("close() must be invoked after all flushed writes are handled.");
        }
        int unflushedCount = this.tail - this.unflushed & this.buffer.length - 1;
        try {
            for (int i2 = 0; i2 < unflushedCount; ++i2) {
                Entry e2 = this.buffer[this.unflushed + i2 & this.buffer.length - 1];
                ChannelOutboundBuffer.safeRelease(e2.msg);
                e2.msg = null;
                ChannelOutboundBuffer.safeFail(e2.promise, cause);
                e2.promise = null;
                int size = e2.pendingSize;
                long oldValue = this.totalPendingSize;
                long newWriteBufferSize = oldValue - (long)size;
                while (!TOTAL_PENDING_SIZE_UPDATER.compareAndSet(this, oldValue, newWriteBufferSize)) {
                    oldValue = this.totalPendingSize;
                    newWriteBufferSize = oldValue - (long)size;
                }
                e2.pendingSize = 0;
            }
        }
        finally {
            this.tail = this.unflushed;
            this.inFail = false;
        }
        this.recycle();
    }

    private static void safeRelease(Object message) {
        try {
            ReferenceCountUtil.release(message);
        }
        catch (Throwable t2) {
            logger.warn("Failed to release a message.", t2);
        }
    }

    private static void safeFail(ChannelPromise promise, Throwable cause) {
        if (!(promise instanceof VoidChannelPromise) && !promise.tryFailure(cause)) {
            logger.warn("Promise done already: {} - new exception is:", (Object)promise, (Object)cause);
        }
    }

    public void recycle() {
        if (this.buffer.length > 32) {
            Entry[] e2 = new Entry[32];
            System.arraycopy(this.buffer, 0, e2, 0, 32);
            this.buffer = e2;
        }
        if (this.nioBuffers.length > 32) {
            this.nioBuffers = new ByteBuffer[32];
        } else {
            Arrays.fill(this.nioBuffers, null);
        }
        this.flushed = 0;
        this.unflushed = 0;
        this.tail = 0;
        this.channel = null;
        RECYCLER.recycle(this, this.handle);
    }

    private static final class Entry {
        Object msg;
        ByteBuffer[] buffers;
        ByteBuffer buf;
        ChannelPromise promise;
        long progress;
        long total;
        int pendingSize;
        int count = -1;

        private Entry() {
        }

        public void clear() {
            this.buffers = null;
            this.buf = null;
            this.msg = null;
            this.promise = null;
            this.progress = 0L;
            this.total = 0L;
            this.pendingSize = 0;
            this.count = -1;
        }
    }
}

