/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import net.minecraft.server.MinecraftServer;

public class bk
extends y {
    @Override
    public String c() {
        return "scoreboard";
    }

    @Override
    public int a() {
        return 2;
    }

    @Override
    public String c(ac ac2) {
        return "commands.scoreboard.usage";
    }

    /*
     * Enabled force condition propagation
     * Lifted jumps to return sites
     */
    @Override
    public void b(ac ac2, String[] stringArray) {
        if (stringArray.length < 1) throw new ci("commands.scoreboard.usage", new Object[0]);
        if (stringArray[0].equalsIgnoreCase("objectives")) {
            if (stringArray.length == 1) {
                throw new ci("commands.scoreboard.objectives.usage", new Object[0]);
            }
            if (stringArray[1].equalsIgnoreCase("list")) {
                this.d(ac2);
                return;
            } else if (stringArray[1].equalsIgnoreCase("add")) {
                if (stringArray.length < 4) throw new ci("commands.scoreboard.objectives.add.usage", new Object[0]);
                this.c(ac2, stringArray, 2);
                return;
            } else if (stringArray[1].equalsIgnoreCase("remove")) {
                if (stringArray.length != 3) throw new ci("commands.scoreboard.objectives.remove.usage", new Object[0]);
                this.h(ac2, stringArray[2]);
                return;
            } else {
                if (!stringArray[1].equalsIgnoreCase("setdisplay")) throw new ci("commands.scoreboard.objectives.usage", new Object[0]);
                if (stringArray.length != 3 && stringArray.length != 4) throw new ci("commands.scoreboard.objectives.setdisplay.usage", new Object[0]);
                this.k(ac2, stringArray, 2);
            }
            return;
        }
        if (stringArray[0].equalsIgnoreCase("players")) {
            if (stringArray.length == 1) {
                throw new ci("commands.scoreboard.players.usage", new Object[0]);
            }
            if (stringArray[1].equalsIgnoreCase("list")) {
                if (stringArray.length > 3) throw new ci("commands.scoreboard.players.list.usage", new Object[0]);
                this.l(ac2, stringArray, 2);
                return;
            } else if (stringArray[1].equalsIgnoreCase("add")) {
                if (stringArray.length != 5) throw new ci("commands.scoreboard.players.add.usage", new Object[0]);
                this.m(ac2, stringArray, 2);
                return;
            } else if (stringArray[1].equalsIgnoreCase("remove")) {
                if (stringArray.length != 5) throw new ci("commands.scoreboard.players.remove.usage", new Object[0]);
                this.m(ac2, stringArray, 2);
                return;
            } else if (stringArray[1].equalsIgnoreCase("set")) {
                if (stringArray.length != 5) throw new ci("commands.scoreboard.players.set.usage", new Object[0]);
                this.m(ac2, stringArray, 2);
                return;
            } else {
                if (!stringArray[1].equalsIgnoreCase("reset")) throw new ci("commands.scoreboard.players.usage", new Object[0]);
                if (stringArray.length != 3) throw new ci("commands.scoreboard.players.reset.usage", new Object[0]);
                this.n(ac2, stringArray, 2);
            }
            return;
        }
        if (!stringArray[0].equalsIgnoreCase("teams")) throw new ci("commands.scoreboard.usage", new Object[0]);
        if (stringArray.length == 1) {
            throw new ci("commands.scoreboard.teams.usage", new Object[0]);
        }
        if (stringArray[1].equalsIgnoreCase("list")) {
            if (stringArray.length > 3) throw new ci("commands.scoreboard.teams.list.usage", new Object[0]);
            this.g(ac2, stringArray, 2);
            return;
        } else if (stringArray[1].equalsIgnoreCase("add")) {
            if (stringArray.length < 3) throw new ci("commands.scoreboard.teams.add.usage", new Object[0]);
            this.d(ac2, stringArray, 2);
            return;
        } else if (stringArray[1].equalsIgnoreCase("remove")) {
            if (stringArray.length != 3) throw new ci("commands.scoreboard.teams.remove.usage", new Object[0]);
            this.f(ac2, stringArray, 2);
            return;
        } else if (stringArray[1].equalsIgnoreCase("empty")) {
            if (stringArray.length != 3) throw new ci("commands.scoreboard.teams.empty.usage", new Object[0]);
            this.j(ac2, stringArray, 2);
            return;
        } else if (stringArray[1].equalsIgnoreCase("join")) {
            if (stringArray.length < 4 && (stringArray.length != 3 || !(ac2 instanceof yz))) throw new ci("commands.scoreboard.teams.join.usage", new Object[0]);
            this.h(ac2, stringArray, 2);
            return;
        } else if (stringArray[1].equalsIgnoreCase("leave")) {
            if (stringArray.length < 3 && !(ac2 instanceof yz)) throw new ci("commands.scoreboard.teams.leave.usage", new Object[0]);
            this.i(ac2, stringArray, 2);
            return;
        } else {
            if (!stringArray[1].equalsIgnoreCase("option")) throw new ci("commands.scoreboard.teams.usage", new Object[0]);
            if (stringArray.length != 4 && stringArray.length != 5) throw new ci("commands.scoreboard.teams.option.usage", new Object[0]);
            this.e(ac2, stringArray, 2);
        }
    }

    protected bac d() {
        return MinecraftServer.I().a(0).W();
    }

    protected azx a(String string, boolean bl2) {
        bac bac2 = this.d();
        azx azx2 = bac2.b(string);
        if (azx2 == null) {
            throw new cd("commands.scoreboard.objectiveNotFound", string);
        }
        if (bl2 && azx2.c().b()) {
            throw new cd("commands.scoreboard.objectiveReadOnly", string);
        }
        return azx2;
    }

    protected azy a(String string) {
        bac bac2 = this.d();
        azy azy2 = bac2.e(string);
        if (azy2 == null) {
            throw new cd("commands.scoreboard.teamNotFound", string);
        }
        return azy2;
    }

    protected void c(ac ac2, String[] stringArray, int n2) {
        String string = stringArray[n2++];
        String string2 = stringArray[n2++];
        bac bac2 = this.d();
        bah bah2 = (bah)bah.a.get(string2);
        if (bah2 == null) {
            throw new ci("commands.scoreboard.objectives.add.wrongType", string2);
        }
        if (bac2.b(string) != null) {
            throw new cd("commands.scoreboard.objectives.add.alreadyExists", string);
        }
        if (string.length() > 16) {
            throw new cf("commands.scoreboard.objectives.add.tooLong", string, 16);
        }
        if (string.length() == 0) {
            throw new ci("commands.scoreboard.objectives.add.usage", new Object[0]);
        }
        if (stringArray.length > n2) {
            String string3 = bk.a(ac2, stringArray, n2).c();
            if (string3.length() > 32) {
                throw new cf("commands.scoreboard.objectives.add.displayTooLong", string3, 32);
            }
            if (string3.length() > 0) {
                bac2.a(string, bah2).a(string3);
            } else {
                bac2.a(string, bah2);
            }
        } else {
            bac2.a(string, bah2);
        }
        bk.a(ac2, (aa)this, "commands.scoreboard.objectives.add.success", string);
    }

    protected void d(ac ac2, String[] stringArray, int n2) {
        String string = stringArray[n2++];
        bac bac2 = this.d();
        if (bac2.e(string) != null) {
            throw new cd("commands.scoreboard.teams.add.alreadyExists", string);
        }
        if (string.length() > 16) {
            throw new cf("commands.scoreboard.teams.add.tooLong", string, 16);
        }
        if (string.length() == 0) {
            throw new ci("commands.scoreboard.teams.add.usage", new Object[0]);
        }
        if (stringArray.length > n2) {
            String string2 = bk.a(ac2, stringArray, n2).c();
            if (string2.length() > 32) {
                throw new cf("commands.scoreboard.teams.add.displayTooLong", string2, 32);
            }
            if (string2.length() > 0) {
                bac2.f(string).a(string2);
            } else {
                bac2.f(string);
            }
        } else {
            bac2.f(string);
        }
        bk.a(ac2, (aa)this, "commands.scoreboard.teams.add.success", string);
    }

    protected void e(ac ac2, String[] stringArray, int n2) {
        String string;
        azy azy2;
        if ((azy2 = this.a(stringArray[n2++])) == null) {
            return;
        }
        if (!((string = stringArray[n2++].toLowerCase()).equalsIgnoreCase("color") || string.equalsIgnoreCase("friendlyfire") || string.equalsIgnoreCase("seeFriendlyInvisibles"))) {
            throw new ci("commands.scoreboard.teams.option.usage", new Object[0]);
        }
        if (stringArray.length == 4) {
            if (string.equalsIgnoreCase("color")) {
                throw new ci("commands.scoreboard.teams.option.noValue", string, bk.a(a.a(true, false)));
            }
            if (string.equalsIgnoreCase("friendlyfire") || string.equalsIgnoreCase("seeFriendlyInvisibles")) {
                throw new ci("commands.scoreboard.teams.option.noValue", string, bk.a(Arrays.asList("true", "false")));
            }
            throw new ci("commands.scoreboard.teams.option.usage", new Object[0]);
        }
        String string2 = stringArray[n2++];
        if (string.equalsIgnoreCase("color")) {
            a a2 = a.b(string2);
            if (a2 == null || a2.b()) {
                throw new ci("commands.scoreboard.teams.option.noValue", string, bk.a(a.a(true, false)));
            }
            azy2.b(a2.toString());
            azy2.c(a.v.toString());
        } else if (string.equalsIgnoreCase("friendlyfire")) {
            if (!string2.equalsIgnoreCase("true") && !string2.equalsIgnoreCase("false")) {
                throw new ci("commands.scoreboard.teams.option.noValue", string, bk.a(Arrays.asList("true", "false")));
            }
            azy2.a(string2.equalsIgnoreCase("true"));
        } else if (string.equalsIgnoreCase("seeFriendlyInvisibles")) {
            if (!string2.equalsIgnoreCase("true") && !string2.equalsIgnoreCase("false")) {
                throw new ci("commands.scoreboard.teams.option.noValue", string, bk.a(Arrays.asList("true", "false")));
            }
            azy2.b(string2.equalsIgnoreCase("true"));
        }
        bk.a(ac2, (aa)this, "commands.scoreboard.teams.option.success", string, azy2.b(), string2);
    }

    protected void f(ac ac2, String[] stringArray, int n2) {
        azy azy2;
        bac bac2 = this.d();
        if ((azy2 = this.a(stringArray[n2++])) == null) {
            return;
        }
        bac2.d(azy2);
        bk.a(ac2, (aa)this, "commands.scoreboard.teams.remove.success", azy2.b());
    }

    /*
     * Enabled force condition propagation
     * Lifted jumps to return sites
     */
    protected void g(ac ac2, String[] stringArray, int n2) {
        bac bac2 = this.d();
        if (stringArray.length > n2) {
            azy azy2;
            if ((azy2 = this.a(stringArray[n2++])) == null) {
                return;
            }
            Collection collection = azy2.d();
            if (collection.size() <= 0) throw new cd("commands.scoreboard.teams.list.player.empty", azy2.b());
            fr fr2 = new fr("commands.scoreboard.teams.list.player.count", collection.size(), azy2.b());
            fr2.b().a(a.c);
            ac2.a(fr2);
            ac2.a(new fq(bk.a(collection.toArray())));
            return;
        } else {
            Collection collection = bac2.g();
            if (collection.size() <= 0) throw new cd("commands.scoreboard.teams.list.empty", new Object[0]);
            fr fr3 = new fr("commands.scoreboard.teams.list.count", collection.size());
            fr3.b().a(a.c);
            ac2.a(fr3);
            for (azy azy3 : collection) {
                ac2.a(new fr("commands.scoreboard.teams.list.entry", azy3.b(), azy3.c(), azy3.d().size()));
            }
        }
    }

    protected void h(ac ac2, String[] stringArray, int n2) {
        bac bac2 = this.d();
        String string = stringArray[n2++];
        HashSet<String> hashSet = new HashSet<String>();
        HashSet<String> hashSet2 = new HashSet<String>();
        if (ac2 instanceof yz && n2 == stringArray.length) {
            String string2 = bk.b(ac2).b_();
            if (bac2.a(string2, string)) {
                hashSet.add(string2);
            } else {
                hashSet2.add(string2);
            }
        } else {
            while (n2 < stringArray.length) {
                String string3;
                if (bac2.a(string3 = bk.e(ac2, stringArray[n2++]), string)) {
                    hashSet.add(string3);
                    continue;
                }
                hashSet2.add(string3);
            }
        }
        if (!hashSet.isEmpty()) {
            bk.a(ac2, (aa)this, "commands.scoreboard.teams.join.success", hashSet.size(), string, bk.a(hashSet.toArray(new String[0])));
        }
        if (!hashSet2.isEmpty()) {
            throw new cd("commands.scoreboard.teams.join.failure", hashSet2.size(), string, bk.a(hashSet2.toArray(new String[0])));
        }
    }

    protected void i(ac ac2, String[] stringArray, int n2) {
        bac bac2 = this.d();
        HashSet<String> hashSet = new HashSet<String>();
        HashSet<String> hashSet2 = new HashSet<String>();
        if (ac2 instanceof yz && n2 == stringArray.length) {
            String string = bk.b(ac2).b_();
            if (bac2.g(string)) {
                hashSet.add(string);
            } else {
                hashSet2.add(string);
            }
        } else {
            while (n2 < stringArray.length) {
                String string;
                if (bac2.g(string = bk.e(ac2, stringArray[n2++]))) {
                    hashSet.add(string);
                    continue;
                }
                hashSet2.add(string);
            }
        }
        if (!hashSet.isEmpty()) {
            bk.a(ac2, (aa)this, "commands.scoreboard.teams.leave.success", hashSet.size(), bk.a(hashSet.toArray(new String[0])));
        }
        if (!hashSet2.isEmpty()) {
            throw new cd("commands.scoreboard.teams.leave.failure", hashSet2.size(), bk.a(hashSet2.toArray(new String[0])));
        }
    }

    protected void j(ac ac2, String[] stringArray, int n2) {
        azy azy2;
        bac bac2 = this.d();
        if ((azy2 = this.a(stringArray[n2++])) == null) {
            return;
        }
        ArrayList arrayList = new ArrayList(azy2.d());
        if (arrayList.isEmpty()) {
            throw new cd("commands.scoreboard.teams.empty.alreadyEmpty", azy2.b());
        }
        for (String string : arrayList) {
            bac2.a(string, azy2);
        }
        bk.a(ac2, (aa)this, "commands.scoreboard.teams.empty.success", arrayList.size(), azy2.b());
    }

    protected void h(ac ac2, String string) {
        bac bac2 = this.d();
        azx azx2 = this.a(string, false);
        bac2.k(azx2);
        bk.a(ac2, (aa)this, "commands.scoreboard.objectives.remove.success", string);
    }

    protected void d(ac ac2) {
        bac bac2 = this.d();
        Collection collection = bac2.c();
        if (collection.size() > 0) {
            fr fr2 = new fr("commands.scoreboard.objectives.list.count", collection.size());
            fr2.b().a(a.c);
            ac2.a(fr2);
            for (azx azx2 : collection) {
                ac2.a(new fr("commands.scoreboard.objectives.list.entry", azx2.b(), azx2.d(), azx2.c().a()));
            }
        } else {
            throw new cd("commands.scoreboard.objectives.list.empty", new Object[0]);
        }
    }

    protected void k(ac ac2, String[] stringArray, int n2) {
        bac bac2 = this.d();
        String string = stringArray[n2++];
        int n3 = bac.j(string);
        azx azx2 = null;
        if (stringArray.length == 4) {
            azx2 = this.a(stringArray[n2++], false);
        }
        if (n3 < 0) {
            throw new cd("commands.scoreboard.objectives.setdisplay.invalidSlot", string);
        }
        bac2.a(n3, azx2);
        if (azx2 != null) {
            bk.a(ac2, (aa)this, "commands.scoreboard.objectives.setdisplay.successSet", bac.b(n3), azx2.b());
        } else {
            bk.a(ac2, (aa)this, "commands.scoreboard.objectives.setdisplay.successCleared", bac.b(n3));
        }
    }

    /*
     * Enabled force condition propagation
     * Lifted jumps to return sites
     */
    protected void l(ac ac2, String[] stringArray, int n2) {
        bac bac2 = this.d();
        if (stringArray.length > n2) {
            String string;
            Map map;
            if ((map = bac2.d(string = bk.e(ac2, stringArray[n2++]))).size() <= 0) throw new cd("commands.scoreboard.players.list.player.empty", string);
            fr fr2 = new fr("commands.scoreboard.players.list.player.count", map.size(), string);
            fr2.b().a(a.c);
            ac2.a(fr2);
            for (azz azz2 : map.values()) {
                ac2.a(new fr("commands.scoreboard.players.list.player.entry", azz2.c(), azz2.d().d(), azz2.d().b()));
            }
            return;
        } else {
            Collection collection = bac2.d();
            if (collection.size() <= 0) throw new cd("commands.scoreboard.players.list.empty", new Object[0]);
            fr fr3 = new fr("commands.scoreboard.players.list.count", collection.size());
            fr3.b().a(a.c);
            ac2.a(fr3);
            ac2.a(new fq(bk.a(collection.toArray())));
        }
    }

    protected void m(ac ac2, String[] stringArray, int n2) {
        String string = stringArray[n2 - 1];
        String string2 = bk.e(ac2, stringArray[n2++]);
        azx azx2 = this.a(stringArray[n2++], true);
        int n3 = string.equalsIgnoreCase("set") ? bk.a(ac2, stringArray[n2++]) : bk.a(ac2, stringArray[n2++], 1);
        bac bac2 = this.d();
        azz azz2 = bac2.a(string2, azx2);
        if (string.equalsIgnoreCase("set")) {
            azz2.c(n3);
        } else if (string.equalsIgnoreCase("add")) {
            azz2.a(n3);
        } else {
            azz2.b(n3);
        }
        bk.a(ac2, (aa)this, "commands.scoreboard.players.set.success", azx2.b(), string2, azz2.c());
    }

    protected void n(ac ac2, String[] stringArray, int n2) {
        bac bac2 = this.d();
        String string = bk.e(ac2, stringArray[n2++]);
        bac2.c(string);
        bk.a(ac2, (aa)this, "commands.scoreboard.players.reset.success", string);
    }

    @Override
    public List a(ac ac2, String[] stringArray) {
        if (stringArray.length == 1) {
            return bk.a(stringArray, "objectives", "players", "teams");
        }
        if (stringArray[0].equalsIgnoreCase("objectives")) {
            if (stringArray.length == 2) {
                return bk.a(stringArray, "list", "add", "remove", "setdisplay");
            }
            if (stringArray[1].equalsIgnoreCase("add")) {
                if (stringArray.length == 4) {
                    Set set = bah.a.keySet();
                    return bk.a(stringArray, set);
                }
            } else if (stringArray[1].equalsIgnoreCase("remove")) {
                if (stringArray.length == 3) {
                    return bk.a(stringArray, this.a(false));
                }
            } else if (stringArray[1].equalsIgnoreCase("setdisplay")) {
                if (stringArray.length == 3) {
                    return bk.a(stringArray, "list", "sidebar", "belowName");
                }
                if (stringArray.length == 4) {
                    return bk.a(stringArray, this.a(false));
                }
            }
        } else if (stringArray[0].equalsIgnoreCase("players")) {
            if (stringArray.length == 2) {
                return bk.a(stringArray, "set", "add", "remove", "reset", "list");
            }
            if (stringArray[1].equalsIgnoreCase("set") || stringArray[1].equalsIgnoreCase("add") || stringArray[1].equalsIgnoreCase("remove")) {
                if (stringArray.length == 3) {
                    return bk.a(stringArray, MinecraftServer.I().E());
                }
                if (stringArray.length == 4) {
                    return bk.a(stringArray, this.a(true));
                }
            } else if ((stringArray[1].equalsIgnoreCase("reset") || stringArray[1].equalsIgnoreCase("list")) && stringArray.length == 3) {
                return bk.a(stringArray, this.d().d());
            }
        } else if (stringArray[0].equalsIgnoreCase("teams")) {
            if (stringArray.length == 2) {
                return bk.a(stringArray, "add", "remove", "join", "leave", "empty", "list", "option");
            }
            if (stringArray[1].equalsIgnoreCase("join")) {
                if (stringArray.length == 3) {
                    return bk.a(stringArray, this.d().f());
                }
                if (stringArray.length >= 4) {
                    return bk.a(stringArray, MinecraftServer.I().E());
                }
            } else {
                if (stringArray[1].equalsIgnoreCase("leave")) {
                    return bk.a(stringArray, MinecraftServer.I().E());
                }
                if (stringArray[1].equalsIgnoreCase("empty") || stringArray[1].equalsIgnoreCase("list") || stringArray[1].equalsIgnoreCase("remove")) {
                    if (stringArray.length == 3) {
                        return bk.a(stringArray, this.d().f());
                    }
                } else if (stringArray[1].equalsIgnoreCase("option")) {
                    if (stringArray.length == 3) {
                        return bk.a(stringArray, this.d().f());
                    }
                    if (stringArray.length == 4) {
                        return bk.a(stringArray, "color", "friendlyfire", "seeFriendlyInvisibles");
                    }
                    if (stringArray.length == 5) {
                        if (stringArray[3].equalsIgnoreCase("color")) {
                            return bk.a(stringArray, a.a(true, false));
                        }
                        if (stringArray[3].equalsIgnoreCase("friendlyfire") || stringArray[3].equalsIgnoreCase("seeFriendlyInvisibles")) {
                            return bk.a(stringArray, "true", "false");
                        }
                    }
                }
            }
        }
        return null;
    }

    protected List a(boolean bl2) {
        Collection collection = this.d().c();
        ArrayList<String> arrayList = new ArrayList<String>();
        for (azx azx2 : collection) {
            if (bl2 && azx2.c().b()) continue;
            arrayList.add(azx2.b());
        }
        return arrayList;
    }

    @Override
    public boolean a(String[] stringArray, int n2) {
        if (stringArray[0].equalsIgnoreCase("players")) {
            return n2 == 2;
        }
        if (stringArray[0].equalsIgnoreCase("teams")) {
            return n2 == 2 || n2 == 3;
        }
        return false;
    }
}

