/*
 * Decompiled with CFR 0.152.
 */
import com.google.common.base.Predicate;

final class oa
implements Predicate {
    oa() {
    }

    public boolean a(String string) {
        return !qn.b(string);
    }

    public /* synthetic */ boolean apply(Object object) {
        return this.a((String)object);
    }
}

