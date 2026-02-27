/*
 * Decompiled with CFR 0.152.
 */
public class alr
extends aor {
    private add a;

    @Override
    public void a(dh dh2) {
        super.a(dh2);
        if (dh2.b("RecordItem", 10)) {
            this.a(add.a(dh2.m("RecordItem")));
        } else if (dh2.f("Record") > 0) {
            this.a(new add(adb.d(dh2.f("Record")), 1, 0));
        }
    }

    @Override
    public void b(dh dh2) {
        super.b(dh2);
        if (this.a() != null) {
            dh2.a("RecordItem", this.a().b(new dh()));
            dh2.a("Record", adb.b(this.a().b()));
        }
    }

    public add a() {
        return this.a;
    }

    public void a(add add2) {
        this.a = add2;
        this.e();
    }
}

