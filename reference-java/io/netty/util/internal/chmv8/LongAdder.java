/*
 * Decompiled with CFR 0.152.
 */
package io.netty.util.internal.chmv8;

import io.netty.util.internal.chmv8.Striped64;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.Serializable;

public class LongAdder
extends Striped64
implements Serializable {
    private static final long serialVersionUID = 7249069246863182397L;

    @Override
    final long fn(long v, long x2) {
        return v + x2;
    }

    public void add(long x2) {
        long b2;
        Striped64.Cell[] as2 = this.cells;
        if (this.cells != null || !this.casBase(b2 = this.base, b2 + x2)) {
            long v;
            Striped64.Cell a2;
            int n2;
            boolean uncontended = true;
            Striped64.HashCode hc2 = (Striped64.HashCode)threadHashCode.get();
            int h2 = hc2.code;
            if (as2 == null || (n2 = as2.length) < 1 || (a2 = as2[n2 - 1 & h2]) == null || !(uncontended = a2.cas(v = a2.value, v + x2))) {
                this.retryUpdate(x2, hc2, uncontended);
            }
        }
    }

    public void increment() {
        this.add(1L);
    }

    public void decrement() {
        this.add(-1L);
    }

    public long sum() {
        long sum = this.base;
        Striped64.Cell[] as2 = this.cells;
        if (as2 != null) {
            for (Striped64.Cell a2 : as2) {
                if (a2 == null) continue;
                sum += a2.value;
            }
        }
        return sum;
    }

    public void reset() {
        this.internalReset(0L);
    }

    public long sumThenReset() {
        long sum = this.base;
        Striped64.Cell[] as2 = this.cells;
        this.base = 0L;
        if (as2 != null) {
            for (Striped64.Cell a2 : as2) {
                if (a2 == null) continue;
                sum += a2.value;
                a2.value = 0L;
            }
        }
        return sum;
    }

    public String toString() {
        return Long.toString(this.sum());
    }

    @Override
    public long longValue() {
        return this.sum();
    }

    @Override
    public int intValue() {
        return (int)this.sum();
    }

    @Override
    public float floatValue() {
        return this.sum();
    }

    @Override
    public double doubleValue() {
        return this.sum();
    }

    private void writeObject(ObjectOutputStream s2) throws IOException {
        s2.defaultWriteObject();
        s2.writeLong(this.sum());
    }

    private void readObject(ObjectInputStream s2) throws IOException, ClassNotFoundException {
        s2.defaultReadObject();
        this.busy = 0;
        this.cells = null;
        this.base = s2.readLong();
    }
}

