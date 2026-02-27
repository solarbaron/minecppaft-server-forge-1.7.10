/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.UUID;

public class il
extends ft {
    private int a;
    private final List b = new ArrayList();

    public il() {
    }

    public il(int n2, Collection collection) {
        this.a = n2;
        for (ti ti2 : collection) {
            this.b.add(new im(this, ti2.a().a(), ti2.b(), ti2.c()));
        }
    }

    @Override
    public void a(et et2) {
        this.a = et2.readInt();
        int n2 = et2.readInt();
        for (int i2 = 0; i2 < n2; ++i2) {
            String string = et2.c(64);
            double d2 = et2.readDouble();
            ArrayList<tj> arrayList = new ArrayList<tj>();
            int n3 = et2.readShort();
            for (int i3 = 0; i3 < n3; ++i3) {
                UUID uUID = new UUID(et2.readLong(), et2.readLong());
                arrayList.add(new tj(uUID, "Unknown synced attribute modifier", et2.readDouble(), et2.readByte()));
            }
            this.b.add(new im(this, string, d2, arrayList));
        }
    }

    @Override
    public void b(et et2) {
        et2.writeInt(this.a);
        et2.writeInt(this.b.size());
        for (im im2 : this.b) {
            et2.a(im2.a());
            et2.writeDouble(im2.b());
            et2.writeShort(im2.c().size());
            for (tj tj2 : im2.c()) {
                et2.writeLong(tj2.a().getMostSignificantBits());
                et2.writeLong(tj2.a().getLeastSignificantBits());
                et2.writeDouble(tj2.d());
                et2.writeByte(tj2.c());
            }
        }
    }

    public void a(fv fv2) {
        fv2.a(this);
    }
}

