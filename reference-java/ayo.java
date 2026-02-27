/*
 * Decompiled with CFR 0.152.
 */
import java.io.File;
import java.io.FilenameFilter;

class ayo
implements FilenameFilter {
    final /* synthetic */ ayn a;

    ayo(ayn ayn2) {
        this.a = ayn2;
    }

    @Override
    public boolean accept(File file, String string) {
        return string.endsWith(".mcr");
    }
}

