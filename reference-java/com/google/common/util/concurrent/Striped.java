/*
 * Decompiled with CFR 0.152.
 */
package com.google.common.util.concurrent;

import com.google.common.annotations.Beta;
import com.google.common.base.Objects;
import com.google.common.base.Preconditions;
import com.google.common.base.Supplier;
import com.google.common.collect.Iterables;
import com.google.common.collect.MapMaker;
import com.google.common.math.IntMath;
import java.math.RoundingMode;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.Semaphore;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

@Beta
public abstract class Striped<L> {
    private static final Supplier<ReadWriteLock> READ_WRITE_LOCK_SUPPLIER = new Supplier<ReadWriteLock>(){

        @Override
        public ReadWriteLock get() {
            return new ReentrantReadWriteLock();
        }
    };
    private static final int ALL_SET = -1;

    private Striped() {
    }

    public abstract L get(Object var1);

    public abstract L getAt(int var1);

    abstract int indexFor(Object var1);

    public abstract int size();

    public Iterable<L> bulkGet(Iterable<?> keys) {
        int i2;
        Object[] array = Iterables.toArray(keys, Object.class);
        int[] stripes = new int[array.length];
        for (i2 = 0; i2 < array.length; ++i2) {
            stripes[i2] = this.indexFor(array[i2]);
        }
        Arrays.sort(stripes);
        for (i2 = 0; i2 < array.length; ++i2) {
            array[i2] = this.getAt(stripes[i2]);
        }
        List<Object> asList = Arrays.asList(array);
        return Collections.unmodifiableList(asList);
    }

    public static Striped<Lock> lock(int stripes) {
        return new CompactStriped<Lock>(stripes, new Supplier<Lock>(){

            @Override
            public Lock get() {
                return new PaddedLock();
            }
        });
    }

    public static Striped<Lock> lazyWeakLock(int stripes) {
        return new LazyStriped<Lock>(stripes, new Supplier<Lock>(){

            @Override
            public Lock get() {
                return new ReentrantLock(false);
            }
        });
    }

    public static Striped<Semaphore> semaphore(int stripes, final int permits) {
        return new CompactStriped<Semaphore>(stripes, new Supplier<Semaphore>(){

            @Override
            public Semaphore get() {
                return new PaddedSemaphore(permits);
            }
        });
    }

    public static Striped<Semaphore> lazyWeakSemaphore(int stripes, final int permits) {
        return new LazyStriped<Semaphore>(stripes, new Supplier<Semaphore>(){

            @Override
            public Semaphore get() {
                return new Semaphore(permits, false);
            }
        });
    }

    public static Striped<ReadWriteLock> readWriteLock(int stripes) {
        return new CompactStriped<ReadWriteLock>(stripes, READ_WRITE_LOCK_SUPPLIER);
    }

    public static Striped<ReadWriteLock> lazyWeakReadWriteLock(int stripes) {
        return new LazyStriped<ReadWriteLock>(stripes, READ_WRITE_LOCK_SUPPLIER);
    }

    private static int ceilToPowerOfTwo(int x2) {
        return 1 << IntMath.log2(x2, RoundingMode.CEILING);
    }

    private static int smear(int hashCode) {
        hashCode ^= hashCode >>> 20 ^ hashCode >>> 12;
        return hashCode ^ hashCode >>> 7 ^ hashCode >>> 4;
    }

    private static class PaddedSemaphore
    extends Semaphore {
        long q1;
        long q2;
        long q3;

        PaddedSemaphore(int permits) {
            super(permits, false);
        }
    }

    private static class PaddedLock
    extends ReentrantLock {
        long q1;
        long q2;
        long q3;

        PaddedLock() {
            super(false);
        }
    }

    private static class LazyStriped<L>
    extends PowerOfTwoStriped<L> {
        final ConcurrentMap<Integer, L> locks;
        final Supplier<L> supplier;
        final int size;

        LazyStriped(int stripes, Supplier<L> supplier) {
            super(stripes);
            this.size = this.mask == -1 ? Integer.MAX_VALUE : this.mask + 1;
            this.supplier = supplier;
            this.locks = new MapMaker().weakValues().makeMap();
        }

        @Override
        public L getAt(int index) {
            Object existing;
            if (this.size != Integer.MAX_VALUE) {
                Preconditions.checkElementIndex(index, this.size());
            }
            if ((existing = this.locks.get(index)) != null) {
                return (L)existing;
            }
            L created = this.supplier.get();
            existing = this.locks.putIfAbsent(index, created);
            return (L)Objects.firstNonNull(existing, created);
        }

        @Override
        public int size() {
            return this.size;
        }
    }

    private static class CompactStriped<L>
    extends PowerOfTwoStriped<L> {
        private final Object[] array;

        private CompactStriped(int stripes, Supplier<L> supplier) {
            super(stripes);
            Preconditions.checkArgument(stripes <= 0x40000000, "Stripes must be <= 2^30)");
            this.array = new Object[this.mask + 1];
            for (int i2 = 0; i2 < this.array.length; ++i2) {
                this.array[i2] = supplier.get();
            }
        }

        @Override
        public L getAt(int index) {
            return (L)this.array[index];
        }

        @Override
        public int size() {
            return this.array.length;
        }
    }

    private static abstract class PowerOfTwoStriped<L>
    extends Striped<L> {
        final int mask;

        PowerOfTwoStriped(int stripes) {
            Preconditions.checkArgument(stripes > 0, "Stripes must be positive");
            this.mask = stripes > 0x40000000 ? -1 : Striped.ceilToPowerOfTwo(stripes) - 1;
        }

        @Override
        final int indexFor(Object key) {
            int hash = Striped.smear(key.hashCode());
            return hash & this.mask;
        }

        @Override
        public final L get(Object key) {
            return this.getAt(this.indexFor(key));
        }
    }
}

