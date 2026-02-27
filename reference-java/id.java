/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.Collection;

public class id
extends ft {
    private String a = "";
    private String b = "";
    private String c = "";
    private String d = "";
    private Collection e = new ArrayList();
    private int f;
    private int g;

    public id() {
    }

    public id(azy azy2, int n2) {
        this.a = azy2.b();
        this.f = n2;
        if (n2 == 0 || n2 == 2) {
            this.b = azy2.c();
            this.c = azy2.e();
            this.d = azy2.f();
            this.g = azy2.i();
        }
        if (n2 == 0) {
            this.e.addAll(azy2.d());
        }
    }

    public id(azy azy2, Collection collection, int n2) {
        if (n2 != 3 && n2 != 4) {
            throw new IllegalArgumentException("Method must be join or leave for player constructor");
        }
        if (collection == null || collection.isEmpty()) {
            throw new IllegalArgumentException("Players cannot be null/empty");
        }
        this.f = n2;
        this.a = azy2.b();
        this.e.addAll(collection);
    }

    @Override
    public void a(et et2) {
        this.a = et2.c(16);
        this.f = et2.readByte();
        if (this.f == 0 || this.f == 2) {
            this.b = et2.c(32);
            this.c = et2.c(16);
            this.d = et2.c(16);
            this.g = et2.readByte();
        }
        if (this.f == 0 || this.f == 3 || this.f == 4) {
            int n2 = et2.readShort();
            for (int i2 = 0; i2 < n2; ++i2) {
                this.e.add(et2.c(40));
            }
        }
    }

    @Override
    public void b(et et2) {
        et2.a(this.a);
        et2.writeByte(this.f);
        if (this.f == 0 || this.f == 2) {
            et2.a(this.b);
            et2.a(this.c);
            et2.a(this.d);
            et2.writeByte(this.g);
        }
        if (this.f == 0 || this.f == 3 || this.f == 4) {
            et2.writeShort(this.e.size());
            for (String string : this.e) {
                et2.a(string);
            }
        }
    }

    public void a(fv fv2) {
        fv2.a(this);
    }
}

