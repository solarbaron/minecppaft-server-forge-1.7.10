/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.List;

public class ajf {
    private ahb b;
    private int c;
    private int d;
    private int e;
    private final boolean f;
    private List g = new ArrayList();
    final /* synthetic */ aje a;

    public ajf(aje aje2, ahb ahb2, int n2, int n3, int n4) {
        this.a = aje2;
        this.b = ahb2;
        this.c = n2;
        this.d = n3;
        this.e = n4;
        aji aji2 = ahb2.a(n2, n3, n4);
        int n5 = ahb2.e(n2, n3, n4);
        if (((aje)aji2).a) {
            this.f = true;
            n5 &= 0xFFFFFFF7;
        } else {
            this.f = false;
        }
        this.a(n5);
    }

    private void a(int n2) {
        this.g.clear();
        if (n2 == 0) {
            this.g.add(new agt(this.c, this.d, this.e - 1));
            this.g.add(new agt(this.c, this.d, this.e + 1));
        } else if (n2 == 1) {
            this.g.add(new agt(this.c - 1, this.d, this.e));
            this.g.add(new agt(this.c + 1, this.d, this.e));
        } else if (n2 == 2) {
            this.g.add(new agt(this.c - 1, this.d, this.e));
            this.g.add(new agt(this.c + 1, this.d + 1, this.e));
        } else if (n2 == 3) {
            this.g.add(new agt(this.c - 1, this.d + 1, this.e));
            this.g.add(new agt(this.c + 1, this.d, this.e));
        } else if (n2 == 4) {
            this.g.add(new agt(this.c, this.d + 1, this.e - 1));
            this.g.add(new agt(this.c, this.d, this.e + 1));
        } else if (n2 == 5) {
            this.g.add(new agt(this.c, this.d, this.e - 1));
            this.g.add(new agt(this.c, this.d + 1, this.e + 1));
        } else if (n2 == 6) {
            this.g.add(new agt(this.c + 1, this.d, this.e));
            this.g.add(new agt(this.c, this.d, this.e + 1));
        } else if (n2 == 7) {
            this.g.add(new agt(this.c - 1, this.d, this.e));
            this.g.add(new agt(this.c, this.d, this.e + 1));
        } else if (n2 == 8) {
            this.g.add(new agt(this.c - 1, this.d, this.e));
            this.g.add(new agt(this.c, this.d, this.e - 1));
        } else if (n2 == 9) {
            this.g.add(new agt(this.c + 1, this.d, this.e));
            this.g.add(new agt(this.c, this.d, this.e - 1));
        }
    }

    private void b() {
        for (int i2 = 0; i2 < this.g.size(); ++i2) {
            ajf ajf2 = this.a((agt)this.g.get(i2));
            if (ajf2 == null || !ajf2.a(this)) {
                this.g.remove(i2--);
                continue;
            }
            this.g.set(i2, new agt(ajf2.c, ajf2.d, ajf2.e));
        }
    }

    private boolean a(int n2, int n3, int n4) {
        if (aje.b_(this.b, n2, n3, n4)) {
            return true;
        }
        if (aje.b_(this.b, n2, n3 + 1, n4)) {
            return true;
        }
        return aje.b_(this.b, n2, n3 - 1, n4);
    }

    private ajf a(agt agt2) {
        if (aje.b_(this.b, agt2.a, agt2.b, agt2.c)) {
            return new ajf(this.a, this.b, agt2.a, agt2.b, agt2.c);
        }
        if (aje.b_(this.b, agt2.a, agt2.b + 1, agt2.c)) {
            return new ajf(this.a, this.b, agt2.a, agt2.b + 1, agt2.c);
        }
        if (aje.b_(this.b, agt2.a, agt2.b - 1, agt2.c)) {
            return new ajf(this.a, this.b, agt2.a, agt2.b - 1, agt2.c);
        }
        return null;
    }

    private boolean a(ajf ajf2) {
        for (int i2 = 0; i2 < this.g.size(); ++i2) {
            agt agt2 = (agt)this.g.get(i2);
            if (agt2.a != ajf2.c || agt2.c != ajf2.e) continue;
            return true;
        }
        return false;
    }

    private boolean b(int n2, int n3, int n4) {
        for (int i2 = 0; i2 < this.g.size(); ++i2) {
            agt agt2 = (agt)this.g.get(i2);
            if (agt2.a != n2 || agt2.c != n4) continue;
            return true;
        }
        return false;
    }

    protected int a() {
        int n2 = 0;
        if (this.a(this.c, this.d, this.e - 1)) {
            ++n2;
        }
        if (this.a(this.c, this.d, this.e + 1)) {
            ++n2;
        }
        if (this.a(this.c - 1, this.d, this.e)) {
            ++n2;
        }
        if (this.a(this.c + 1, this.d, this.e)) {
            ++n2;
        }
        return n2;
    }

    private boolean b(ajf ajf2) {
        if (this.a(ajf2)) {
            return true;
        }
        if (this.g.size() == 2) {
            return false;
        }
        if (this.g.isEmpty()) {
            return true;
        }
        return true;
    }

    private void c(ajf ajf2) {
        this.g.add(new agt(ajf2.c, ajf2.d, ajf2.e));
        boolean bl2 = this.b(this.c, this.d, this.e - 1);
        boolean bl3 = this.b(this.c, this.d, this.e + 1);
        boolean bl4 = this.b(this.c - 1, this.d, this.e);
        boolean bl5 = this.b(this.c + 1, this.d, this.e);
        int n2 = -1;
        if (bl2 || bl3) {
            n2 = 0;
        }
        if (bl4 || bl5) {
            n2 = 1;
        }
        if (!this.f) {
            if (bl3 && bl5 && !bl2 && !bl4) {
                n2 = 6;
            }
            if (bl3 && bl4 && !bl2 && !bl5) {
                n2 = 7;
            }
            if (bl2 && bl4 && !bl3 && !bl5) {
                n2 = 8;
            }
            if (bl2 && bl5 && !bl3 && !bl4) {
                n2 = 9;
            }
        }
        if (n2 == 0) {
            if (aje.b_(this.b, this.c, this.d + 1, this.e - 1)) {
                n2 = 4;
            }
            if (aje.b_(this.b, this.c, this.d + 1, this.e + 1)) {
                n2 = 5;
            }
        }
        if (n2 == 1) {
            if (aje.b_(this.b, this.c + 1, this.d + 1, this.e)) {
                n2 = 2;
            }
            if (aje.b_(this.b, this.c - 1, this.d + 1, this.e)) {
                n2 = 3;
            }
        }
        if (n2 < 0) {
            n2 = 0;
        }
        int n3 = n2;
        if (this.f) {
            n3 = this.b.e(this.c, this.d, this.e) & 8 | n2;
        }
        this.b.a(this.c, this.d, this.e, n3, 3);
    }

    private boolean c(int n2, int n3, int n4) {
        ajf ajf2 = this.a(new agt(n2, n3, n4));
        if (ajf2 == null) {
            return false;
        }
        ajf2.b();
        return ajf2.b(this);
    }

    public void a(boolean bl2, boolean bl3) {
        boolean bl4 = this.c(this.c, this.d, this.e - 1);
        boolean bl5 = this.c(this.c, this.d, this.e + 1);
        boolean bl6 = this.c(this.c - 1, this.d, this.e);
        boolean bl7 = this.c(this.c + 1, this.d, this.e);
        int n2 = -1;
        if ((bl4 || bl5) && !bl6 && !bl7) {
            n2 = 0;
        }
        if ((bl6 || bl7) && !bl4 && !bl5) {
            n2 = 1;
        }
        if (!this.f) {
            if (bl5 && bl7 && !bl4 && !bl6) {
                n2 = 6;
            }
            if (bl5 && bl6 && !bl4 && !bl7) {
                n2 = 7;
            }
            if (bl4 && bl6 && !bl5 && !bl7) {
                n2 = 8;
            }
            if (bl4 && bl7 && !bl5 && !bl6) {
                n2 = 9;
            }
        }
        if (n2 == -1) {
            if (bl4 || bl5) {
                n2 = 0;
            }
            if (bl6 || bl7) {
                n2 = 1;
            }
            if (!this.f) {
                if (bl2) {
                    if (bl5 && bl7) {
                        n2 = 6;
                    }
                    if (bl6 && bl5) {
                        n2 = 7;
                    }
                    if (bl7 && bl4) {
                        n2 = 9;
                    }
                    if (bl4 && bl6) {
                        n2 = 8;
                    }
                } else {
                    if (bl4 && bl6) {
                        n2 = 8;
                    }
                    if (bl7 && bl4) {
                        n2 = 9;
                    }
                    if (bl6 && bl5) {
                        n2 = 7;
                    }
                    if (bl5 && bl7) {
                        n2 = 6;
                    }
                }
            }
        }
        if (n2 == 0) {
            if (aje.b_(this.b, this.c, this.d + 1, this.e - 1)) {
                n2 = 4;
            }
            if (aje.b_(this.b, this.c, this.d + 1, this.e + 1)) {
                n2 = 5;
            }
        }
        if (n2 == 1) {
            if (aje.b_(this.b, this.c + 1, this.d + 1, this.e)) {
                n2 = 2;
            }
            if (aje.b_(this.b, this.c - 1, this.d + 1, this.e)) {
                n2 = 3;
            }
        }
        if (n2 < 0) {
            n2 = 0;
        }
        this.a(n2);
        int n3 = n2;
        if (this.f) {
            n3 = this.b.e(this.c, this.d, this.e) & 8 | n2;
        }
        if (bl3 || this.b.e(this.c, this.d, this.e) != n3) {
            this.b.a(this.c, this.d, this.e, n3, 3);
            for (int i2 = 0; i2 < this.g.size(); ++i2) {
                ajf ajf2 = this.a((agt)this.g.get(i2));
                if (ajf2 == null) continue;
                ajf2.b();
                if (!ajf2.b(this)) continue;
                ajf2.c(this);
            }
        }
    }
}

