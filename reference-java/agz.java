/*
 * Decompiled with CFR 0.152.
 */
class agz {
    private String a;
    private boolean b;
    private int c;
    private double d;

    public agz(String string) {
        this.a(string);
    }

    public void a(String string) {
        this.a = string;
        this.b = Boolean.parseBoolean(string);
        try {
            this.c = Integer.parseInt(string);
        }
        catch (NumberFormatException numberFormatException) {
            // empty catch block
        }
        try {
            this.d = Double.parseDouble(string);
        }
        catch (NumberFormatException numberFormatException) {
            // empty catch block
        }
    }

    public String a() {
        return this.a;
    }

    public boolean b() {
        return this.b;
    }
}

