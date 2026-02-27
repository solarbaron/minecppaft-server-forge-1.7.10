/*
 * Decompiled with CFR 0.152.
 */
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

class lv
extends Thread {
    final /* synthetic */ lt a;

    lv(lt lt2, String string) {
        this.a = lt2;
        super(string);
    }

    @Override
    public void run() {
        BufferedReader bufferedReader = new BufferedReader(new InputStreamReader(System.in));
        try {
            String string;
            while (!this.a.ag() && this.a.q() && (string = bufferedReader.readLine()) != null) {
                this.a.a(string, this.a);
            }
        }
        catch (IOException iOException) {
            lt.aF().error("Exception handling console input", (Throwable)iOException);
        }
    }
}

