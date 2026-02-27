/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;
import org.apache.commons.lang3.ObjectUtils;

public class te {
    private final sa a;
    private boolean b = true;
    private static final HashMap c = new HashMap();
    private final Map d = new HashMap();
    private boolean e;
    private ReadWriteLock f = new ReentrantReadWriteLock();

    public te(sa sa2) {
        this.a = sa2;
    }

    public void a(int n2, Object object) {
        Integer n3 = (Integer)c.get(object.getClass());
        if (n3 == null) {
            throw new IllegalArgumentException("Unknown data type: " + object.getClass());
        }
        if (n2 > 31) {
            throw new IllegalArgumentException("Data value id is too big with " + n2 + "! (Max is " + 31 + ")");
        }
        if (this.d.containsKey(n2)) {
            throw new IllegalArgumentException("Duplicate id value for " + n2 + "!");
        }
        tf tf2 = new tf(n3, n2, object);
        this.f.writeLock().lock();
        this.d.put(n2, tf2);
        this.f.writeLock().unlock();
        this.b = false;
    }

    public void a(int n2, int n3) {
        tf tf2 = new tf(n3, n2, null);
        this.f.writeLock().lock();
        this.d.put(n2, tf2);
        this.f.writeLock().unlock();
        this.b = false;
    }

    public byte a(int n2) {
        return (Byte)this.i(n2).b();
    }

    public short b(int n2) {
        return (Short)this.i(n2).b();
    }

    public int c(int n2) {
        return (Integer)this.i(n2).b();
    }

    public float d(int n2) {
        return ((Float)this.i(n2).b()).floatValue();
    }

    public String e(int n2) {
        return (String)this.i(n2).b();
    }

    public add f(int n2) {
        return (add)this.i(n2).b();
    }

    private tf i(int n2) {
        tf tf2;
        this.f.readLock().lock();
        try {
            tf2 = (tf)this.d.get(n2);
        }
        catch (Throwable throwable) {
            b b2 = b.a(throwable, "Getting synched entity data");
            k k2 = b2.a("Synched entity data");
            k2.a("Data ID", n2);
            throw new s(b2);
        }
        this.f.readLock().unlock();
        return tf2;
    }

    public void b(int n2, Object object) {
        tf tf2 = this.i(n2);
        if (ObjectUtils.notEqual(object, tf2.b())) {
            tf2.a(object);
            this.a.i(n2);
            tf2.a(true);
            this.e = true;
        }
    }

    public void h(int n2) {
        tf.a(this.i(n2), true);
        this.e = true;
    }

    public boolean a() {
        return this.e;
    }

    public static void a(List list, et et2) {
        if (list != null) {
            for (tf tf2 : list) {
                te.a(et2, tf2);
            }
        }
        et2.writeByte(127);
    }

    public List b() {
        ArrayList<tf> arrayList = null;
        if (this.e) {
            this.f.readLock().lock();
            for (tf tf2 : this.d.values()) {
                if (!tf2.d()) continue;
                tf2.a(false);
                if (arrayList == null) {
                    arrayList = new ArrayList<tf>();
                }
                arrayList.add(tf2);
            }
            this.f.readLock().unlock();
        }
        this.e = false;
        return arrayList;
    }

    public void a(et et2) {
        this.f.readLock().lock();
        for (tf tf2 : this.d.values()) {
            te.a(et2, tf2);
        }
        this.f.readLock().unlock();
        et2.writeByte(127);
    }

    public List c() {
        ArrayList<tf> arrayList = null;
        this.f.readLock().lock();
        for (tf tf2 : this.d.values()) {
            if (arrayList == null) {
                arrayList = new ArrayList<tf>();
            }
            arrayList.add(tf2);
        }
        this.f.readLock().unlock();
        return arrayList;
    }

    private static void a(et et2, tf tf2) {
        int n2 = (tf2.c() << 5 | tf2.a() & 0x1F) & 0xFF;
        et2.writeByte(n2);
        switch (tf2.c()) {
            case 0: {
                et2.writeByte(((Byte)tf2.b()).byteValue());
                break;
            }
            case 1: {
                et2.writeShort(((Short)tf2.b()).shortValue());
                break;
            }
            case 2: {
                et2.writeInt((Integer)tf2.b());
                break;
            }
            case 3: {
                et2.writeFloat(((Float)tf2.b()).floatValue());
                break;
            }
            case 4: {
                et2.a((String)tf2.b());
                break;
            }
            case 5: {
                add add2 = (add)tf2.b();
                et2.a(add2);
                break;
            }
            case 6: {
                r r2 = (r)tf2.b();
                et2.writeInt(r2.a);
                et2.writeInt(r2.b);
                et2.writeInt(r2.c);
            }
        }
    }

    public static List b(et et2) {
        ArrayList<tf> arrayList = null;
        byte by2 = et2.readByte();
        while (by2 != 127) {
            if (arrayList == null) {
                arrayList = new ArrayList<tf>();
            }
            int n2 = (by2 & 0xE0) >> 5;
            int n3 = by2 & 0x1F;
            tf tf2 = null;
            switch (n2) {
                case 0: {
                    tf2 = new tf(n2, n3, et2.readByte());
                    break;
                }
                case 1: {
                    tf2 = new tf(n2, n3, et2.readShort());
                    break;
                }
                case 2: {
                    tf2 = new tf(n2, n3, et2.readInt());
                    break;
                }
                case 3: {
                    tf2 = new tf(n2, n3, Float.valueOf(et2.readFloat()));
                    break;
                }
                case 4: {
                    tf2 = new tf(n2, n3, et2.c(Short.MAX_VALUE));
                    break;
                }
                case 5: {
                    tf2 = new tf(n2, n3, et2.c());
                    break;
                }
                case 6: {
                    int n4 = et2.readInt();
                    int n5 = et2.readInt();
                    int n6 = et2.readInt();
                    tf2 = new tf(n2, n3, new r(n4, n5, n6));
                }
            }
            arrayList.add(tf2);
            by2 = et2.readByte();
        }
        return arrayList;
    }

    public boolean d() {
        return this.b;
    }

    public void e() {
        this.e = false;
    }

    static {
        c.put(Byte.class, 0);
        c.put(Short.class, 1);
        c.put(Integer.class, 2);
        c.put(Float.class, 3);
        c.put(String.class, 4);
        c.put(add.class, 5);
        c.put(r.class, 6);
    }
}

