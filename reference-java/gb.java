/*
 * Decompiled with CFR 0.152.
 */
import com.mojang.authlib.GameProfile;
import com.mojang.authlib.properties.Property;
import java.util.List;
import java.util.UUID;

public class gb
extends ft {
    private int a;
    private GameProfile b;
    private int c;
    private int d;
    private int e;
    private byte f;
    private byte g;
    private int h;
    private te i;
    private List j;

    public gb() {
    }

    public gb(yz yz2) {
        this.a = yz2.y();
        this.b = yz2.bJ();
        this.c = qh.c(yz2.s * 32.0);
        this.d = qh.c(yz2.t * 32.0);
        this.e = qh.c(yz2.u * 32.0);
        this.f = (byte)(yz2.y * 256.0f / 360.0f);
        this.g = (byte)(yz2.z * 256.0f / 360.0f);
        add add2 = yz2.bm.h();
        this.h = add2 == null ? 0 : adb.b(add2.b());
        this.i = yz2.z();
    }

    @Override
    public void a(et et2) {
        this.a = et2.a();
        UUID uUID = UUID.fromString(et2.c(36));
        this.b = new GameProfile(uUID, et2.c(16));
        int n2 = et2.a();
        for (int i2 = 0; i2 < n2; ++i2) {
            String string = et2.c(Short.MAX_VALUE);
            String string2 = et2.c(Short.MAX_VALUE);
            String string3 = et2.c(Short.MAX_VALUE);
            this.b.getProperties().put(string, new Property(string, string2, string3));
        }
        this.c = et2.readInt();
        this.d = et2.readInt();
        this.e = et2.readInt();
        this.f = et2.readByte();
        this.g = et2.readByte();
        this.h = et2.readShort();
        this.j = te.b(et2);
    }

    @Override
    public void b(et et2) {
        et2.b(this.a);
        UUID uUID = this.b.getId();
        et2.a(uUID == null ? "" : uUID.toString());
        et2.a(this.b.getName());
        et2.b(this.b.getProperties().size());
        for (Property property : this.b.getProperties().values()) {
            et2.a(property.getName());
            et2.a(property.getValue());
            et2.a(property.getSignature());
        }
        et2.writeInt(this.c);
        et2.writeInt(this.d);
        et2.writeInt(this.e);
        et2.writeByte(this.f);
        et2.writeByte(this.g);
        et2.writeShort(this.h);
        this.i.a(et2);
    }

    public void a(fv fv2) {
        fv2.a(this);
    }

    @Override
    public String b() {
        return String.format("id=%d, gameProfile='%s', x=%.2f, y=%.2f, z=%.2f, carried=%d", this.a, this.b, Float.valueOf((float)this.c / 32.0f), Float.valueOf((float)this.d / 32.0f), Float.valueOf((float)this.e / 32.0f), this.h);
    }
}

