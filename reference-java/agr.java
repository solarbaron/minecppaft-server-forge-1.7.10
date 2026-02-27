/*
 * Decompiled with CFR 0.152.
 */
public class agr
extends qw {
    public final dh b;
    public final String c;
    final /* synthetic */ agq d;

    public agr(agq agq2, dh dh2) {
        this.d = agq2;
        super(dh2.f("Weight"));
        dh dh3 = dh2.m("Properties");
        String string = dh2.j("Type");
        if (string.equals("Minecart")) {
            if (dh3 != null) {
                switch (dh3.f("Type")) {
                    case 1: {
                        string = "MinecartChest";
                        break;
                    }
                    case 2: {
                        string = "MinecartFurnace";
                        break;
                    }
                    case 0: {
                        string = "MinecartRideable";
                    }
                }
            } else {
                string = "MinecartRideable";
            }
        }
        this.b = dh3;
        this.c = string;
    }

    public agr(agq agq2, dh dh2, String string) {
        this.d = agq2;
        super(1);
        if (string.equals("Minecart")) {
            if (dh2 != null) {
                switch (dh2.f("Type")) {
                    case 1: {
                        string = "MinecartChest";
                        break;
                    }
                    case 2: {
                        string = "MinecartFurnace";
                        break;
                    }
                    case 0: {
                        string = "MinecartRideable";
                    }
                }
            } else {
                string = "MinecartRideable";
            }
        }
        this.b = dh2;
        this.c = string;
    }

    public dh a() {
        dh dh2 = new dh();
        dh2.a("Properties", this.b);
        dh2.a("Type", this.c);
        dh2.a("Weight", this.a);
        return dh2;
    }
}

