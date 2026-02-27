/*
 * Decompiled with CFR 0.152.
 */
import com.google.gson.JsonParseException;
import java.util.List;
import net.minecraft.server.MinecraftServer;
import org.apache.commons.lang3.exception.ExceptionUtils;

public class ba
extends y {
    @Override
    public String c() {
        return "tellraw";
    }

    @Override
    public int a() {
        return 2;
    }

    @Override
    public String c(ac ac2) {
        return "commands.tellraw.usage";
    }

    @Override
    public void b(ac ac2, String[] stringArray) {
        if (stringArray.length < 2) {
            throw new ci("commands.tellraw.usage", new Object[0]);
        }
        mw mw2 = ba.d(ac2, stringArray[0]);
        String string = ba.b(ac2, stringArray, 1);
        try {
            fj fj2 = fk.a(string);
            mw2.a(fj2);
        }
        catch (JsonParseException jsonParseException) {
            Throwable throwable = ExceptionUtils.getRootCause(jsonParseException);
            throw new cf("commands.tellraw.jsonException", throwable == null ? "" : throwable.getMessage());
        }
    }

    @Override
    public List a(ac ac2, String[] stringArray) {
        if (stringArray.length == 1) {
            return ba.a(stringArray, MinecraftServer.I().E());
        }
        return null;
    }

    @Override
    public boolean a(String[] stringArray, int n2) {
        return n2 == 0;
    }
}

