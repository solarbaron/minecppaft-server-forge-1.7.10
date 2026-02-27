/*
 * Decompiled with CFR 0.152.
 */
public class bb
extends y {
    @Override
    public String c() {
        return "netstat";
    }

    @Override
    public int a() {
        return 0;
    }

    @Override
    public String c(ac ac2) {
        return "commands.players.usage";
    }

    @Override
    public void b(ac ac2, String[] stringArray) {
        if (ac2 instanceof yz) {
            ac2.a(new fq("Command is not available for players"));
            return;
        }
        if (stringArray.length > 0 && stringArray[0].length() > 1) {
            if ("hottest-read".equals(stringArray[0])) {
                ac2.a(new fq(ej.h.e().toString()));
            } else if ("hottest-write".equals(stringArray[0])) {
                ac2.a(new fq(ej.h.g().toString()));
            } else if ("most-read".equals(stringArray[0])) {
                ac2.a(new fq(ej.h.f().toString()));
            } else if ("most-write".equals(stringArray[0])) {
                ac2.a(new fq(ej.h.h().toString()));
            } else if ("packet-read".equals(stringArray[0])) {
                if (stringArray.length > 1 && stringArray[1].length() > 0) {
                    try {
                        int n2 = Integer.parseInt(stringArray[1].trim());
                        ew ew2 = ej.h.a(n2);
                        this.a(ac2, n2, ew2);
                    }
                    catch (Exception exception) {
                        ac2.a(new fq("Packet " + stringArray[1] + " not found!"));
                    }
                } else {
                    ac2.a(new fq("Packet id is missing"));
                }
            } else if ("packet-write".equals(stringArray[0])) {
                if (stringArray.length > 1 && stringArray[1].length() > 0) {
                    try {
                        int n3 = Integer.parseInt(stringArray[1].trim());
                        ew ew3 = ej.h.b(n3);
                        this.a(ac2, n3, ew3);
                    }
                    catch (Exception exception) {
                        ac2.a(new fq("Packet " + stringArray[1] + " not found!"));
                    }
                } else {
                    ac2.a(new fq("Packet id is missing"));
                }
            } else if ("read-count".equals(stringArray[0])) {
                ac2.a(new fq("total-read-count" + String.valueOf(ej.h.c())));
            } else if ("write-count".equals(stringArray[0])) {
                ac2.a(new fq("total-write-count" + String.valueOf(ej.h.d())));
            } else {
                ac2.a(new fq("Unrecognized: " + stringArray[0]));
            }
        } else {
            String string = "reads: " + ej.h.a();
            string = string + ", writes: " + ej.h.b();
            ac2.a(new fq(string));
        }
    }

    private void a(ac ac2, int n2, ew ew2) {
        if (ew2 != null) {
            ac2.a(new fq(ew2.toString()));
        } else {
            ac2.a(new fq("Packet " + n2 + " not found!"));
        }
    }
}

