/*
 * Decompiled with CFR 0.152.
 */
public class by
extends y {
    @Override
    public String c() {
        return "testfor";
    }

    @Override
    public int a() {
        return 2;
    }

    @Override
    public String c(ac ac2) {
        return "commands.testfor.usage";
    }

    @Override
    public void b(ac ac2, String[] stringArray) {
        if (stringArray.length != 1) {
            throw new ci("commands.testfor.usage", new Object[0]);
        }
        if (!(ac2 instanceof agp)) {
            throw new cd("commands.testfor.failed", new Object[0]);
        }
        by.d(ac2, stringArray[0]);
    }

    @Override
    public boolean a(String[] stringArray, int n2) {
        return n2 == 0;
    }
}

