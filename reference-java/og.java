/*
 * Decompiled with CFR 0.152.
 */
import java.io.File;
import java.io.FilenameFilter;

final class og
implements FilenameFilter {
    og() {
    }

    @Override
    public boolean accept(File file, String string) {
        return string.endsWith(".dat");
    }
}

