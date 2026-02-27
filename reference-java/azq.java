/*
 * Decompiled with CFR 0.152.
 */
import java.io.DataInputStream;
import java.io.DataOutput;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class azq {
    private azc a;
    private Map b = new HashMap();
    private List c = new ArrayList();
    private Map d = new HashMap();

    public azq(azc azc2) {
        this.a = azc2;
        this.b();
    }

    public ayl a(Class clazz, String string) {
        ayl ayl2;
        block7: {
            ayl2 = (ayl)this.b.get(string);
            if (ayl2 != null) {
                return ayl2;
            }
            if (this.a != null) {
                try {
                    File file = this.a.a(string);
                    if (file == null || !file.exists()) break block7;
                    try {
                        ayl2 = (ayl)clazz.getConstructor(String.class).newInstance(string);
                    }
                    catch (Exception exception) {
                        throw new RuntimeException("Failed to instantiate " + clazz.toString(), exception);
                    }
                    FileInputStream fileInputStream = new FileInputStream(file);
                    dh dh2 = du.a(fileInputStream);
                    fileInputStream.close();
                    ayl2.a(dh2.m("data"));
                }
                catch (Exception exception) {
                    exception.printStackTrace();
                }
            }
        }
        if (ayl2 != null) {
            this.b.put(string, ayl2);
            this.c.add(ayl2);
        }
        return ayl2;
    }

    public void a(String string, ayl ayl2) {
        if (ayl2 == null) {
            throw new RuntimeException("Can't set null data");
        }
        if (this.b.containsKey(string)) {
            this.c.remove(this.b.remove(string));
        }
        this.b.put(string, ayl2);
        this.c.add(ayl2);
    }

    public void a() {
        for (int i2 = 0; i2 < this.c.size(); ++i2) {
            ayl ayl2 = (ayl)this.c.get(i2);
            if (!ayl2.d()) continue;
            this.a(ayl2);
            ayl2.a(false);
        }
    }

    private void a(ayl ayl2) {
        if (this.a == null) {
            return;
        }
        try {
            File file = this.a.a(ayl2.h);
            if (file != null) {
                dh dh2 = new dh();
                ayl2.b(dh2);
                dh dh3 = new dh();
                dh3.a("data", dh2);
                FileOutputStream fileOutputStream = new FileOutputStream(file);
                du.a(dh3, fileOutputStream);
                fileOutputStream.close();
            }
        }
        catch (Exception exception) {
            exception.printStackTrace();
        }
    }

    private void b() {
        try {
            this.d.clear();
            if (this.a == null) {
                return;
            }
            File file = this.a.a("idcounts");
            if (file != null && file.exists()) {
                DataInputStream dataInputStream = new DataInputStream(new FileInputStream(file));
                dh dh2 = du.a(dataInputStream);
                dataInputStream.close();
                for (String string : dh2.c()) {
                    dy dy2 = dh2.a(string);
                    if (!(dy2 instanceof dw)) continue;
                    dw dw2 = (dw)dy2;
                    String string2 = string;
                    short s2 = dw2.e();
                    this.d.put(string2, s2);
                }
            }
        }
        catch (Exception exception) {
            exception.printStackTrace();
        }
    }

    public int a(String string) {
        Object object;
        Comparable<Short> comparable;
        Short s2 = (Short)this.d.get(string);
        if (s2 == null) {
            s2 = 0;
        } else {
            comparable = s2;
            s2 = (short)(s2 + 1);
            object = s2;
        }
        this.d.put(string, s2);
        if (this.a == null) {
            return s2.shortValue();
        }
        try {
            comparable = this.a.a("idcounts");
            if (comparable != null) {
                object = new dh();
                for (String string2 : this.d.keySet()) {
                    short s3 = (Short)this.d.get(string2);
                    ((dh)object).a(string2, s3);
                }
                DataOutputStream dataOutputStream = new DataOutputStream(new FileOutputStream((File)comparable));
                du.a((dh)object, (DataOutput)dataOutputStream);
                dataOutputStream.close();
            }
        }
        catch (Exception exception) {
            exception.printStackTrace();
        }
        return s2.shortValue();
    }
}

