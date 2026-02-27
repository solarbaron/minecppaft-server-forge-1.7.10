/*
 * Decompiled with CFR 0.152.
 */
import com.mojang.authlib.GameProfile;

public class kc {
    private final int a;
    private final int b;
    private GameProfile[] c;

    public kc(int n2, int n3) {
        this.a = n2;
        this.b = n3;
    }

    public int a() {
        return this.a;
    }

    public int b() {
        return this.b;
    }

    public GameProfile[] c() {
        return this.c;
    }

    public void a(GameProfile[] gameProfileArray) {
        this.c = gameProfileArray;
    }
}

